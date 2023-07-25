#include "dhcp.h"
#include "defs.h"
#include "arp.h"
#include "ethernet.h"
#include "rtl8139.h"

void dhcp_send_discover() {
    dhcp_packet *packet = kalloc();
    memset(packet, 0, sizeof(dhcp_packet));

    packet->operation = DHCP_DISCOVER;
    packet->hw_type = HARDWARE_TYPE_ETH;
    packet->hw_length = MAC_SIZE;
    packet->hops = 0;

    packet->transaction_id = hton32(XID);

    uint8 dst_ip[4];
    memset(dst_ip, 0xFF, 4);

    packet->secs = 0;
    packet->flags = 0;

    rtl8139_get_mac(packet->client_haddr);

    packet->magic_cookie = 0x63825363;

    uint8* options = packet->options;

    *(options++) = 53;
    *(options++) = 1;
    *(options++) = DHCP_DISCOVER;

    *(options++) = 50;
    *(options++) = 4;
    *((uint32*)(options)) = hton32(0x0a00020e);
    memcpy((uint32*)(options), dst_ip, 4);
    options += 4;

    // dont need all the adresses (because we don't have or don't know)
}