#ifndef UI_H
#define UI_H

#include <stdio.h>

void print_error_msg(FILE* stream, int code);

void print_host_resolving_error_msg(FILE* stream, int getaddrinfo_code);

void print_announce(FILE* stream, const struct addrinfo* addr, int max_hops);

void print_report_for_ttl(FILE* stream, uint8_t ttl, 
                            const struct sockaddr_in* response_srcs, 
                            const ssize_t* timings_nsec, size_t queries_per_ttl);

#endif
