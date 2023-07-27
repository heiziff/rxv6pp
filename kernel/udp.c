#include "udp.h"
#include "dhcp.h"
#include "ip.h"
#include "ethernet.h"
#include "defs.h"


void udp_send_packet(uint8 *dst_ip, uint16 src_port, uint16 dst_port, uint8 *data, uint length) {
    udp_packet *packet = kalloc();
    memset(packet, 0, sizeof(udp_packet));

    packet->src_port = hton16(src_port);
    packet->dst_port = hton16(dst_port);
    packet->length = hton16(length + sizeof(udp_packet));
    // Checksum on IPv4 UDP is optional :)
    packet->checksum = 0;

    // Copy data
    uint8* packet_data = ((uint8*) packet) + sizeof(udp_packet);
    memcpy(packet_data, data, length);

    ip_send_packet(dst_ip, (uint8*) packet, IP_PROTOCOL_UDP, length + sizeof(udp_packet));

    kfree(packet);
}

void udp_recv_packet(udp_packet *packet) {
    uint16 dst_port = ntoh16(packet->dst_port);

    //uint8 *data = ((uint8*) packet) + sizeof(udp_packet);

    if (ntoh16(packet->dst_port) == DHCP_PORT_CLIENT) {
        dhcp_recv_packet(((void*) packet) + sizeof(udp_packet));
    }
    else {
        printk(" Got UDP packet on (unhandled) Port %d\n", dst_port);
    }

    dbg(" Got udp packet with length %d\n", packet->length);
}
