#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "error_codes.h"
#include "ui.h"
#include "icmp_ops.h"

// As in 'original' traceroute
#define DEFAULT_MAX_HOPS 30
#define TTL_LIMIT 255
#define TIMEOUT_MILLIS 3000
#define QUERIES_PER_TTL 3
#define RECV_BUF_SIZE 1500 //Definitely enough for interesting headers

static bool interrupted = false;

void sighandler(int signal) {
    interrupted = true;
}

void free_all_resources(struct addrinfo* addr, int sockfd, 
                        void* icmp_msg_buf1, void* icmp_msg_buf2) {
    freeaddrinfo(addr);
    close(sockfd);
    free(icmp_msg_buf1);
    free(icmp_msg_buf2);
}

int getaddrinfo_needed(const char* node, int protocol, struct addrinfo** result) {
    assert(NULL != node);
    assert(NULL != result);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = protocol;

    return getaddrinfo(node, NULL, &hints, result);
}

int main(int argc, char** argv) {
    if ((2 > argc) || (3 < argc)) {
        print_error_msg(stderr, WRONG_ARGUMENTS_NUMBER);
        return WRONG_ARGUMENTS_NUMBER;
    }

    uint8_t max_hops = DEFAULT_MAX_HOPS;

    if (3 == argc) {
        char* endptr = NULL;
        long max_hops_l = strtol(argv[2], &endptr, 10);
        if (('\0' != *endptr) || (0 >= max_hops_l) || (TTL_LIMIT < max_hops_l)) {
            print_error_msg(stderr, INVALID_ARGUMENT);
            return INVALID_ARGUMENT;
        }
        max_hops = max_hops_l;
    }

    struct protoent* icmp_protoent = getprotobyname("icmp");
    if (NULL == icmp_protoent) {
        print_error_msg(stderr, PROTOCOL_NUMBER_UNKNOWN);
        return PROTOCOL_NUMBER_UNKNOWN;
    }
    int icmp_protocol_number = icmp_protoent->p_proto;

    struct addrinfo* addr_found = NULL;
    int getaddrinfo_result = getaddrinfo_needed(argv[1], icmp_protocol_number,
                                                &addr_found);

    if (0 != getaddrinfo_result) {
        print_host_resolving_error_msg(stderr, getaddrinfo_result);
        freeaddrinfo(addr_found);
        return HOST_RESOLVING_ERROR;
    }

    print_announce(stdout, addr_found, max_hops);

    int sockfd = socket(addr_found->ai_family, addr_found->ai_socktype,
                        addr_found->ai_protocol);
    
    if (0 > sockfd) {
        print_error_msg(stderr, SOCKET_OPENING_ERROR);
        freeaddrinfo(addr_found);
        return SOCKET_OPENING_ERROR;
    }

    size_t icmp_echo_request_len = 0;
    void* icmp_echo_request = NULL;

    int icmp_request_creation_result = create_initial_icmp_echo_request(
                                        &icmp_echo_request, 
                                        &icmp_echo_request_len);
    if (0 != icmp_request_creation_result) {
        print_error_msg(stderr, icmp_request_creation_result);
        freeaddrinfo(addr_found);
        close(sockfd);
        return icmp_request_creation_result;
    }

    const size_t recv_buf_size = RECV_BUF_SIZE;
    void* response_buf = malloc(recv_buf_size);
    if (NULL == response_buf) {
        print_error_msg(stderr, MEM_ALLOCATION_ERROR);
        free_all_resources(addr_found, sockfd, icmp_echo_request, NULL);
        return MEM_ALLOCATION_ERROR;
    }

    struct sigaction signal_action = {0};
    signal_action.sa_handler = sighandler;

    if (sigaction(SIGINT, &signal_action, NULL) ||
            sigaction(SIGTERM, &signal_action, NULL)) {
        print_error_msg(stderr, SIGACTION_ERROR);
        return SIGACTION_ERROR;
    }

    uint8_t ttl = 1;
    struct pollfd socket_pollfd = {
            .fd = sockfd, 
            .events = POLLIN, 
            .revents = 0
    };
    bool reached = false;

    while ((max_hops >= ttl) && !reached) {
        if (interrupted) {
            print_error_msg(stderr, INTERRUPTED);
            free_all_resources(addr_found, sockfd, icmp_echo_request,
                                response_buf);
            return INTERRUPTED;
        }
        int setsockopt_result = 
            setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
        if (0 != setsockopt_result) {
            print_error_msg(stderr, SETTING_TTL_FAILED);
            free_all_resources(addr_found, sockfd, icmp_echo_request, 
                                response_buf);
            return SETTING_TTL_FAILED;
        }
        const int response_timeout = TIMEOUT_MILLIS;
        const size_t queries_per_ttl = QUERIES_PER_TTL;
        struct sockaddr_in response_srcs[queries_per_ttl];
        memset(response_srcs, 0, queries_per_ttl * sizeof(*response_srcs));
        ssize_t timings_nsec[queries_per_ttl];
        memset(timings_nsec, 0, queries_per_ttl * sizeof(*timings_nsec));
        struct timespec begin;
        struct timespec end;
        for (size_t i = 0; i < queries_per_ttl; ++i) {
            while (true) {
                ssize_t bytes_sent = 
                    sendto(sockfd, icmp_echo_request, icmp_echo_request_len, 0, 
                            addr_found->ai_addr, addr_found->ai_addrlen);
                if (interrupted) {
                    print_error_msg(stderr, INTERRUPTED);
                    free_all_resources(addr_found, sockfd, icmp_echo_request,
                                        response_buf);
                    return INTERRUPTED;
                }
                if (icmp_echo_request_len > bytes_sent) {
                    if (EINTR == errno) {
                        /**
                         * A signal arrived, but not one of those setting
                         * interrupted = true or killing the proces - retrying.
                        */
                       continue;
                    }
                    //Socket is blocking, smth happened
                    print_error_msg(stderr, SEND_ERROR);
                    free_all_resources(addr_found, sockfd, icmp_echo_request, 
                                        response_buf);
                    return SEND_ERROR;
                }
                break;
            }
            if (clock_gettime(CLOCK_MONOTONIC_RAW, &begin)) {
                print_error_msg(stderr, CLOCK_ERROR);
                //Not a fatal problem - continue.
                timings_nsec[i] = -1;
            }

            int poll_result = 0;
            while (true) {
                //Multiplexing a single fd for multiplexing - as you've asked
                poll_result = poll(&socket_pollfd, 1, response_timeout);
                if (interrupted) {
                    print_error_msg(stderr, INTERRUPTED);
                    free_all_resources(addr_found, sockfd, icmp_echo_request,
                                        response_buf);
                    return INTERRUPTED;
                }
                if (0 > poll_result) {
                    if (EINTR == errno) {
                        continue;
                    }
                    print_error_msg(stderr, POLL_ERROR);
                    free_all_resources(addr_found, sockfd, icmp_echo_request, 
                                        response_buf);
                    return POLL_ERROR;
                }
                if (0 == poll_result) {
                    //Timeout
                    timings_nsec[i] = -1;
                    break;
                }
                if (POLLIN == (socket_pollfd.revents & POLLIN)) {
                    struct sockaddr_in src_addr;
                    socklen_t addrlen = sizeof(src_addr);
                    ssize_t bytes_read = 0;
                    while (true) {
                        bytes_read = 
                            recvfrom(sockfd, response_buf, recv_buf_size, 0, 
                                (struct sockaddr*)&src_addr, &addrlen);
                        if (interrupted) {
                            print_error_msg(stderr, INTERRUPTED);
                            free_all_resources(addr_found, sockfd, icmp_echo_request,
                                                response_buf);
                            return INTERRUPTED;
                        }
                        if (0 > bytes_read) {
                            if (EINTR == errno) {
                                continue;
                            }
                            print_error_msg(stderr, RECV_ERROR);
                            free_all_resources(addr_found, sockfd, 
                                                icmp_echo_request, response_buf);
                            return RECV_ERROR;
                        }
                        break;
                    }
                    bool is_time_exceeded = false;
                    bool is_echo_response = false;
                    is_response_with_type(icmp_echo_request, response_buf,
                        bytes_read, 
                        ((struct sockaddr_in*)(addr_found->ai_addr))->sin_addr,
                        src_addr.sin_addr, &is_time_exceeded, &is_echo_response);
                    if (is_time_exceeded || is_echo_response) {
                        if (clock_gettime(CLOCK_MONOTONIC_RAW, &end)) {
                            print_error_msg(stderr, CLOCK_ERROR);
                            //Not a fatal problem - continue.
                            timings_nsec[i] = -1;
                        }
                        else {
                            timings_nsec[i] = 
                                1000000000 * (end.tv_sec - begin.tv_sec) + 
                                    end.tv_nsec - begin.tv_nsec;
                        }
                        if (is_echo_response) {
                            reached = true;
                        }
                        response_srcs[i] = src_addr;
                        break;
                    }
                    /**
                     * else continue: we got not the datagram we needed, 
                     * but we still can get the needed one
                    */
                }
            }
            increment_seq_number(icmp_echo_request, icmp_echo_request_len);
        }
        print_report_for_ttl(stderr, ttl, response_srcs, timings_nsec, 
                            queries_per_ttl);
        memset(timings_nsec, 0, queries_per_ttl * sizeof(*timings_nsec));
        memset(response_srcs, 0, queries_per_ttl * sizeof(*response_srcs));
        ttl++;
    }
    
    free_all_resources(addr_found, sockfd, icmp_echo_request, response_buf);
}
