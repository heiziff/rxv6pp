#include "arp.h"
#include "rtl8139.h"
#include "ethernet.h"
#include "defs.h"

#define IPV4_ADDR_SIZE 4

#define ARP_OP_REQUEST 1
#define ARP_OP_REPLY 2

uint8 bcast_haddr[6] = {[0 ... 5] = 0xFF};

void arp_send_packet(uint8 *target_haddr, uint8 *target_paddr) {
    printk(" arp_send: call\n");
    arp_packet *packet = (arp_packet*) kalloc();

    packet->hardware_type = hton16(HARDWARE_TYPE_ETH);
    packet->protocol_type = hton16(PROTOCOL_TYPE_IPV4);

    packet->haddr_len = MAC_SIZE;
    packet->paddr_len = IPV4_ADDR_SIZE;

    // TODO: dont hardcode only arp requests
    packet->operation = hton16(ARP_OP_REQUEST);

    // Set src mac:
    rtl8139_get_mac(packet->sender_haddr);
    printk(" arp_send: Got mac: %p\n", *((uint64*)packet->sender_haddr));

    // Hardcode IP address (I hope this works)
    uint8 ip[4] = {10, 0, 2, 42};
    memcpy(packet->sender_paddr, ip, 4);

    // Fill in target mac
    memcpy(packet->target_haddr, target_haddr, MAC_SIZE);

    // Fill in target protocol addr
    memcpy(packet->target_paddr, target_paddr, IPV4_ADDR_SIZE);

    printk(" apr_send: Done\n");


    ethernet_send_packet(bcast_haddr, (uint8*) packet, ETH_TYPE_ARP, sizeof(arp_packet));

}

uint64 sys_arp(void) {
    uint64 paddr;

    argaddr(0, &paddr);

    arp_send_packet(bcast_haddr, (uint8*) paddr);

    return 0;

}