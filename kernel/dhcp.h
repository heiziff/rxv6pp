#ifndef INCLUDED_DHCP_H
#define INCLUDED_DHCP_H

#include "types.h"

#define DHCP_CLIENT 1
#define DHCP_SERVER 2

#define DHCP_MAGIC 0x63825363

#define DHCP_DISCOVER 1
#define DHCP_OFFER 2
#define DHCP_REQUEST 3
#define DHCP_ACK 5

#define XID 0xFF4242FF

typedef struct __attribute__((__packed__)) dhcp_packet_s {
    uint8 operation;
    uint8 hw_type;
    uint8 hw_length;
    uint8 hops;

    uint32 transaction_id;

    uint16 secs;
    uint16 flags;

    // filled in by client, if he wants to renew his IP
    uint8 client_ip[4];

    // filled in by the server, when IP is assigned
    uint8 your_ip[4];

    // Server fills out
    uint8 server_ip[4];

    // Server fills out if forwarding dhcp messages
    uint8 relay_ip[4];

    // client mac (with padding)
    uint8 client_haddr[16];

    uint8 server_name[64];

    // Boot file (apparently used in combination with server_name)
    uint8 boot_file[128];

    uint32 magic_cookie;

    uint8 options[64];

} dhcp_packet;

void get_ip(uint8* buf);

extern uint8 own_ip[4];

void dhcp_send_discover();

void dhcp_recv_packet(dhcp_packet *packet);

#endif