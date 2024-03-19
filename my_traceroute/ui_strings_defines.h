#ifndef UI_STRINGS_DEFINES_H
#define UI_STRINGS_DEFINES_H

#define USAGE "Need a single mandatory argument - host's name or address, \
and one optional - max hops number (between 1 and 255)\n"

#define INVALID_ARGUMENT_MSG "Got invalid argument\n"

#define SIGACTION_ERROR_MSG_TEMPLATE "Failed setting signal handler: %s\n"

#define PROTOCOL_NUMBER_UNKNOWN_MSG "Could not find protocol number. \
Is ICMP present in /etc/protocols?"

#define HOST_RESOLVING_ERROR_MSG "Could not resolve host\n"

#define HOST_RESOLVING_ERROR_MSG_TEMPLATE "Could not resolve host: %s\n"

#define SOCKET_OPENING_ERROR_MSG_TEMPLATE "Could not open socket: %s\n"

#define MEM_ALLOCATION_ERROR_MSG "Memory allocation error\n"

#define SETTING_TTL_FAILED_MSG_TEMPLATE "Failed to set TTL: %s\n"

#define SEND_ERROR_MSG_TEMPLATE "Failed to send request: %s\n"

#define CLOCK_ERROR_MSG_TEMPLATE "Clock access error: %s\n"

#define POLL_ERROR_MSG_TEMPLATE "Poll failed: %s\n"

#define RECV_ERROR_MSG_TEMPLATE "Receiveing IP dgram failed: %s\n"

#define INTERRUPTED_MSG "Job interrupted by signal, stopping\n"

#define ANNOUNCE_MSG_TEMPLATE "\'traceroute\' to %s (%s), %d hops max\n"

#endif
