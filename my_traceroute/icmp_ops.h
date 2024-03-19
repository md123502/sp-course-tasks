#ifndef ICMP_OPS_H
#define ICMP_OPS_H

#include <stddef.h>
#include <stdbool.h>

int create_initial_icmp_echo_request(void** result, size_t* length);

void increment_seq_number(void* icmp_buf, size_t length);

/**
 * Checks following:
 *  - whether ip_response is a kind of response to icmp_echo_request sent to remote
 *      - if not, both booleans are false
 *      - else, checks whether it is icmp 'echo response' or 'time exceeded'
*/
void is_response_with_type(const void* icmp_echo_request, const void* ip_response, 
                            size_t response_len,
                            struct in_addr remote_addressed,
                            struct in_addr remote_answered,
                            bool* is_time_exceeded_response,
                            bool* is_echo_response);

#endif
