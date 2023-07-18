#ifndef INCLUDED_ETHERNET_H
#define INCLUDED_ETHERNET_H

#include "types.h"

#define MAC_SIZE 6

// constants from https://en.wikipedia.org/wiki/Ethernet_frame
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IP 0x0800

typedef struct __attribute__((__packed__)) ethernet_frame_s {
    uint8 dst_mac[MAC_SIZE];
    uint8 src_mac[MAC_SIZE];
    uint16 type;
    // Data comes here
} ethernet_frame;

uint16 hton16(uint16 h_val);

uint16 ntoh16(uint16 n_val);

void ethernet_send_packet(uint8 *dst_mac, uint8 *data, uint16 type, uint32 length);

void ethernet_recv_packet(ethernet_frame *frame, uint32 length);

#endif