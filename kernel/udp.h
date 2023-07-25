#ifndef INCLUDED_UDP_H
#define INCLUDED_UDP_H

#include "types.h"

#define DHCP_PORT_SERVER 67
#define DHCP_PORT_CLIENT 68

typedef struct udp_packet_s {
    uint16 src_port;
    uint16 dst_port;
    uint16 length;
    uint16 checksum;
} udp_packet;


void udp_send_packet(uint8 *dst_ip, uint16 src_port, uint16 dst_port, uint8 *data, uint length);

void udp_recv_packet();

#endif