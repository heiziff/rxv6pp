
#ifndef INCLUDED_ARP_H
#define INCLUDED_ARP_H

#include "types.h"

#define HARDWARE_TYPE_ETH 0x1
#define PROTOCOL_TYPE_IPV4 0x0800

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

// from https://en.wikipedia.org/wiki/Address_Resolution_Protocol#Packet_structure
typedef struct __attribute__((__packed__)) arp_packet_s {
  uint16 hardware_type;
  uint16 protocol_type;
  uint8 haddr_len;
  uint8 paddr_len;
  uint16 operation;
  uint8 sender_haddr[6];
  uint8 sender_paddr[4];
  uint8 target_haddr[6];
  uint8 target_paddr[4];

} arp_packet;

extern uint8 bcast_haddr[6];

void arp_init();
void arp_send_packet(uint8 *, uint8 *, uint16);
void arp_receive_packet(arp_packet *, int);

uint8 *arp_lookup(uint8* ip_address);

void sleep_for_arp_response(uint8* ip);

#endif
