#ifndef INCLUDED_IP_H
#define INCLUDED_IP_H

#include "types.h"

#define IP_HEADER_SIZE_32 5

#define IP_PROTOCOL_ICMP 0x01
#define IP_PROTOCOL_UDP 0x11

#define IPV4_ADDR_SIZE 4

typedef struct __attribute__((__packed__)) ip_datagram_s {
    uint8 version_headerlength;
    uint8 dscp_ecn;
    uint16 length;
    uint16 id;
    uint16 flags_fragment;
    uint8 ttl;
    uint8 protocol;
    uint16 header_checksum;
    uint8 src_address[4];
    uint8 dst_address[4];
    // no options for now
} ip_datagram;

void ip_send_packet(uint8 *dst_ip, uint8 *data, uint8 protocol, uint32 length);

#endif