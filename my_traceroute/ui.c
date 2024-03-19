#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include "error_codes.h"
#include "ui_strings_defines.h"
#include "ui.h"

#define HOSTNAME_BUF_SIZE 256

void print_error_msg(FILE* stream, int code) {
    assert(NULL != stream);
    switch (code) {
        case WRONG_ARGUMENTS_NUMBER:
            fprintf(stream, USAGE);
            break;
        case INVALID_ARGUMENT: 
            fprintf(stream, INVALID_ARGUMENT_MSG);
            fprintf(stream, USAGE);
            break;
        case SIGACTION_ERROR:
            fprintf(stream, SIGACTION_ERROR_MSG_TEMPLATE, strerror(errno));
            break;
        case PROTOCOL_NUMBER_UNKNOWN:
            fprintf(stream, PROTOCOL_NUMBER_UNKNOWN_MSG);
            break;
        case HOST_RESOLVING_ERROR:
            fprintf(stream, HOST_RESOLVING_ERROR_MSG);
            break;
        case SOCKET_OPENING_ERROR:
            fprintf(stream, SOCKET_OPENING_ERROR_MSG_TEMPLATE, strerror(errno));
            break;
        case MEM_ALLOCATION_ERROR:
            fprintf(stream, MEM_ALLOCATION_ERROR_MSG);
            break;
        case SETTING_TTL_FAILED:
            fprintf(stream, SETTING_TTL_FAILED_MSG_TEMPLATE, strerror(errno));
            break;
        case SEND_ERROR:
            fprintf(stream, SEND_ERROR_MSG_TEMPLATE, strerror(errno));
            break;
        case CLOCK_ERROR:
            fprintf(stream, CLOCK_ERROR_MSG_TEMPLATE, strerror(errno));
            break;
        case POLL_ERROR:
            fprintf(stream, POLL_ERROR_MSG_TEMPLATE, strerror(errno));
            break;
        case RECV_ERROR:
            fprintf(stream, RECV_ERROR_MSG_TEMPLATE, strerror(errno));
            break;
        case INTERRUPTED:
            fprintf(stream, INTERRUPTED_MSG);
            break;
    }
}

void print_host_resolving_error_msg(FILE* stream, int getaddrinfo_code) {
    assert(NULL != stream);
    fprintf(stream, HOST_RESOLVING_ERROR_MSG_TEMPLATE, 
        gai_strerror(getaddrinfo_code));
}

void print_announce(FILE* stream, const struct addrinfo* addr, int max_hops) {
    assert(NULL != stream);
    assert(NULL != addr);
    char str_addr_buf[INET_ADDRSTRLEN];
    void* inet_addr = &((struct sockaddr_in*)addr->ai_addr)->sin_addr;
    const char* ntop_result = inet_ntop(addr->ai_family, inet_addr, 
                                        str_addr_buf, INET_ADDRSTRLEN);
    char* canonname = (NULL != addr->ai_canonname) ? addr->ai_canonname : "";
    char* str_addr = (NULL != ntop_result) ? str_addr_buf : "??";
    fprintf(stream, ANNOUNCE_MSG_TEMPLATE, canonname, str_addr, max_hops);
}

void print_report_for_ttl(FILE* stream, uint8_t ttl, 
                            const struct sockaddr_in* response_srcs, 
                            const ssize_t* timings_nsec, size_t queries_per_ttl) {
    assert(NULL != stream);
    assert(NULL != response_srcs);
    assert(NULL != timings_nsec);
    fprintf(stream, "%3d ", ttl);
    //There is possibility that packets were following different paths
    const struct sockaddr_in* prev_gw = NULL;
    size_t first_ok_id = 0;
    while ((first_ok_id < queries_per_ttl) && (0 >= timings_nsec[first_ok_id])) {
        first_ok_id++;
    }
    if (queries_per_ttl <= first_ok_id) {
        for (size_t j = 0; j < queries_per_ttl; ++j) {
            fprintf(stream, " * ");
        }
        fprintf(stream, "\n");
        return;
    }
    for (size_t i = first_ok_id; i < queries_per_ttl; ++i) {
        if (0 < timings_nsec[i]) {
            if ((NULL == prev_gw) || 
                    (response_srcs[i].sin_addr.s_addr != 
                                prev_gw->sin_addr.s_addr)) {
                char buf[HOSTNAME_BUF_SIZE];
                int getnameinfo_result = 0;
                if (0 == (getnameinfo_result = 
                        getnameinfo((struct sockaddr*)response_srcs + i, 
                                    sizeof(response_srcs[i]),
                                    buf, HOSTNAME_BUF_SIZE, NULL, 0, 0))) {
                    fprintf(stream, "%s ", buf);
                }
                else {
                    fprintf(stream, "%s ", gai_strerror(getnameinfo_result));
                }
                if (NULL != (inet_ntop(AF_INET, &(response_srcs[i].sin_addr), 
                                        buf, HOSTNAME_BUF_SIZE))) {
                    fprintf(stream, "(%s) ", buf);
                }
                else {
                    fprintf(stream, "(%s) ", strerror(errno));
                }
            }
            if ((NULL == prev_gw) && (0 != first_ok_id)) {
                for (size_t j = 0; j < first_ok_id; ++j) {
                    fprintf(stream, " * ");
                }
            }
            fprintf(stream, " %ld.%ldms ", 
                timings_nsec[i] / 1000000, (timings_nsec[i] % 1000000) / 1000);
            prev_gw = response_srcs + i;
        }
        else {
            fprintf(stream, " * ");
        }
    }
    fprintf(stream, "\n");
}
