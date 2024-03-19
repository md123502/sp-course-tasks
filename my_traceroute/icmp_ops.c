#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>

#include "error_codes.h"
#include "icmp_ops.h"

/**
 * Of course, if it was ok to assume running this code on a kind of
 * 'usual' architecture, these ops could be written in more beautiful
 * way by casting 'void*' to a sort of 'struct icmp_header*' and other types,
 * but there is no any standard making _Alignof(type) == sizeof(type) 
 * obligatory for implementation, so, unless such assumption 
 * is allowed to be made, it is safer to extract needed values byte-by-byte
*/

// As in 'original' traceroute
#define DEFAULT_LENGTH 60

#define ICMP_TYPE_OFFSET 0
#define ICMP_ECHO_REQ_TYPE 8
#define ICMP_ECHO_RESP_TYPE 0
#define ICMP_TIME_EXCEEDED_TYPE 11

#define ICMP_CHECKSUM_OFFSET 2

#define ICMP_ECHO_ID_OFFSET 4

#define ICMP_ECHO_SEQ_NUM_OFFSET 6

#define ICMP_ECHO_DATA_OFFSET 8
#define ICMP_ECHO_DATA_OFFSET_FROM_HEADER 4

#define IP_HEADER_LEN 20
#define DEST_ADDR_IN_IP_OFFSET 16
#define ICMP_HEADER_LEN 4
#define ORIGINAL_DGRAM_IN_ICMP_TIME_EXCEEDED_OFFSET ICMP_HEADER_LEN+4

static void set_icmp_type(void* buf, uint8_t type);
static uint8_t get_icmp_type(const void* buf);
static void set_random_icmp_echo_id(void* buf);
static uint16_t get_icmp_echo_id(const void* buf);
static void set_icmp_echo_seq_num(void* buf, uint16_t seq_num);
static uint16_t get_icmp_echo_seq_num(const void* buf);
static void fill_icmp_echo_data_sequentially(void* buf, size_t size);
static void update_icmp_checksum(void* buf, size_t length);
static void set_icmp_checksum(void* buf, uint16_t value);

static void* get_icmp_from_ip(const void* ip_dgram);
static void* get_ret_ip_dg_from_icmp_time_exc_resp(const void* icmp_dgram);

static uint32_t get_ip_dest_from_ip_dgram(const void* ip_dgram);

int create_initial_icmp_echo_request(void** result, size_t* length) {
    assert(NULL != result);
    assert(NULL != length);
    size_t size = DEFAULT_LENGTH;
    uint8_t* buf = malloc(size);
    if (NULL == buf) {
        return MEM_ALLOCATION_ERROR;
    }
    memset(buf, 0, size);
    set_icmp_type(buf, ICMP_ECHO_REQ_TYPE);
    set_random_icmp_echo_id(buf);
    set_icmp_echo_seq_num(buf, 1);
    fill_icmp_echo_data_sequentially(buf, size);
    update_icmp_checksum(buf, size);
    *result = buf;
    *length = size;
    return 0;
}

void increment_seq_number(void* icmp_buf, size_t length) {
    assert(NULL != icmp_buf);
    uint16_t old_seq_num = get_icmp_echo_seq_num(icmp_buf);
    set_icmp_echo_seq_num(icmp_buf, old_seq_num + 1);
    update_icmp_checksum(icmp_buf, length);
}

void is_response_with_type(const void* icmp_echo_request, const void* ip_response,
                            size_t response_len,
                            struct in_addr remote_addressed,
                            struct in_addr remote_answered,
                            bool* is_time_exceeded_response,
                            bool* is_echo_response) {
    assert(NULL != icmp_echo_request);
    assert(NULL != ip_response);
    assert(NULL != is_time_exceeded_response);
    assert(NULL != is_echo_response);
    *is_time_exceeded_response = false;
    *is_echo_response = false;
    size_t minimum_valid_msg_size = IP_HEADER_LEN + ICMP_HEADER_LEN;
    if (minimum_valid_msg_size > response_len) {
        return;
    }
    const void* icmp_response = get_icmp_from_ip(ip_response);
    uint8_t icmp_type = get_icmp_type(icmp_response);
    switch (icmp_type) {
        case ICMP_TIME_EXCEEDED_TYPE:
            minimum_valid_msg_size += IP_HEADER_LEN + ICMP_HEADER_LEN;
            if (minimum_valid_msg_size > response_len) {
                return;
            }
            //Must have headers of an original request
            //If it is response to our request, must have original destination here
            const void* original_ip_dgram_returned = 
                    get_ret_ip_dg_from_icmp_time_exc_resp(icmp_response);
            uint32_t dest_from_returned_ip_dgram = 
                    get_ip_dest_from_ip_dgram(original_ip_dgram_returned);
            if (dest_from_returned_ip_dgram != remote_addressed.s_addr) {
                return;
            }
            const void* original_icmp_dgram_returned = 
                    get_icmp_from_ip(original_ip_dgram_returned);
            uint8_t icmp_type_returned = get_icmp_type(original_icmp_dgram_returned);
            if (ICMP_ECHO_REQ_TYPE != icmp_type_returned) {
                return;
            }
            uint16_t echo_id_returned = get_icmp_echo_id(original_icmp_dgram_returned);
            uint16_t echo_seq_returned = get_icmp_echo_seq_num(original_icmp_dgram_returned);
            if ((get_icmp_echo_id(icmp_echo_request) == echo_id_returned) &&
                    (get_icmp_echo_seq_num(icmp_echo_request) == echo_seq_returned)) {
                *is_time_exceeded_response = true;
            }
            break;
        case ICMP_ECHO_RESP_TYPE:
            minimum_valid_msg_size += ICMP_ECHO_DATA_OFFSET_FROM_HEADER;
            if (minimum_valid_msg_size > response_len) {
                return;
            }
            if (remote_addressed.s_addr != remote_answered.s_addr) {
                return;
            }
            uint16_t echo_id_in_response = get_icmp_echo_id(icmp_response);
            uint16_t echo_seq_in_response = get_icmp_echo_seq_num(icmp_response);
            if ((get_icmp_echo_id(icmp_echo_request) == echo_id_in_response) &&
                    (get_icmp_echo_seq_num(icmp_echo_request) == echo_seq_in_response)) {
                *is_echo_response = true;
            }
            break;
        default:
            break;
    }
}

static void set_icmp_type(void* buf, uint8_t type) {
    assert(NULL != buf);
    ((uint8_t*)buf)[ICMP_TYPE_OFFSET] = type;
}

static uint8_t get_icmp_type(const void* buf) {
    assert(NULL != buf);
    uint8_t type = *(((uint8_t*)buf + ICMP_TYPE_OFFSET));
    return type;
}

static void set_random_icmp_echo_id(void* buf) {
    assert(NULL != buf);
    srand(time(NULL));
    uint16_t id = rand();
    *((uint16_t*)((uint8_t*)buf + ICMP_ECHO_ID_OFFSET)) = htons(id);
}

static uint16_t get_icmp_echo_id(const void* buf) {
    assert(NULL != buf);
    uint16_t echo_id = *((uint16_t*)((uint8_t*)buf + ICMP_ECHO_ID_OFFSET));
    return ntohs(echo_id);
}

static void set_icmp_echo_seq_num(void* buf, uint16_t seq_num) {
    assert(NULL != buf);
    *((uint16_t*)((uint8_t*)buf + ICMP_ECHO_SEQ_NUM_OFFSET)) = htons(seq_num);
}

static uint16_t get_icmp_echo_seq_num(const void* buf) {
    assert(NULL != buf);
    uint16_t seq_num = *((uint16_t*)((uint8_t*)buf + ICMP_ECHO_SEQ_NUM_OFFSET));
    return ntohs(seq_num);
}

static void fill_icmp_echo_data_sequentially(void* buf, size_t size) {
    assert(NULL != buf);
    for (size_t i = ICMP_ECHO_DATA_OFFSET; i < size; ++i) {
        ((uint8_t*)buf)[i] = i;
    }
}

/**
 * From RFC 792 (ICMP):
 *  "The checksum is the 16-bit ones's complement of the one's
 *  complement sum of the ICMP message starting with the ICMP Type.
 *  For computing the checksum, the checksum field should be zero."
*/
static void update_icmp_checksum(void* buf, size_t length) {
    assert(NULL != buf);
    set_icmp_checksum(buf, 0);
    uint8_t* buf_bytes = (uint8_t*) buf;
    uint32_t checksum = 0;
    size_t i = length;
    //length can be odd
    while (1 < i) {
        checksum += *((uint16_t*)buf_bytes);
        buf_bytes += 2;
        i -= 2;
    }

    if (0 < i) {
        //We have that last single byte
        uint16_t padded_byte = (*buf_bytes) << 8;
        checksum += padded_byte;
    }

    while (0 != (checksum >> 16)) {
        //Folding overflows
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }
    checksum = ~checksum;
    set_icmp_checksum(buf, checksum);
}

static void set_icmp_checksum(void* buf, uint16_t value) {
    assert(NULL != buf);
    *((uint16_t*)((uint8_t*)buf + ICMP_CHECKSUM_OFFSET)) = value;
}

static void* get_icmp_from_ip(const void* ip_dgram) {
    assert(NULL != ip_dgram);
    return (uint8_t*)ip_dgram + IP_HEADER_LEN;
}

static void* get_ret_ip_dg_from_icmp_time_exc_resp(const void* icmp_dgram) {
    assert(NULL != icmp_dgram);
    return (uint8_t*)icmp_dgram + ORIGINAL_DGRAM_IN_ICMP_TIME_EXCEEDED_OFFSET;
}

static uint32_t get_ip_dest_from_ip_dgram(const void* ip_dgram) {
    assert(NULL != ip_dgram);
    return *((uint32_t*)((uint8_t*)ip_dgram + DEST_ADDR_IN_IP_OFFSET));
}

