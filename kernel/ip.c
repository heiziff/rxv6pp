#include "ip.h"
#include "ethernet.h"
#include "defs.h"
#include "arp.h"

uint8 own_ip[4] = {10, 0, 2, 42};

uint16 ip_checksum(ip_datagram* header) {
    void** kit = (void**) &header;

    uint16 *it = *kit;

    uint32 sum = 0;
    for(int i = 0; i < IP_HEADER_SIZE_32 * 2; i++) {
        sum += hton16(it[i]);
    }
    int carry = sum >> 16;
    sum = sum & 0x0FFFF;
    sum += carry;

    return (~sum);
}

uint8* get_ip(){
    return own_ip;
}

void ip_send_packet(uint8 *dst_ip, uint8 *data, uint8 protocol, uint32 length) {
    ip_datagram *packet = kalloc();
    memset(packet, 0, IP_HEADER_SIZE_32 * 4);

    // TODO: does this have to be flipped?
    packet->version_headerlength = (IP_HEADER_SIZE_32 << 4) | 4;

    packet->length = hton16(IP_HEADER_SIZE_32 * 4 + length);

    // identification only on fragment
    packet->ttl = 32;

    packet->protocol = protocol;

    // TODO: reverse bytes?
    memcpy(packet->src_address, get_ip(), IPV4_ADDR_SIZE);

    memcpy(packet->dst_address, dst_ip, IPV4_ADDR_SIZE);


    // Lastly, compute checksum
    packet->header_checksum = hton16(ip_checksum(packet));

    // now, copy data
    uint8 *datagram_data = ((uint8*) packet) + IP_HEADER_SIZE_32 * 4;
    memcpy(datagram_data, data, length);

    //Check, if mac address is known:
    uint8 *mac_addr = arp_lookup(dst_ip);
    if (!mac_addr) {
        // if mac is unknown, get ready to ARP:
        // TODO: maybe try more than once?
        arp_send_packet(bcast_haddr, dst_ip);

        // sleep until response
        sleep_for_arp_response(dst_ip);

        // After wakeup: lookup and send:
        mac_addr = arp_lookup(dst_ip);

        if (!mac_addr) {
            printk(" IP_send: couldn't find out target hwaddr!\n");
            return;
        }
    }

    // MAC is known, send ethernet packet
    ethernet_send_packet(mac_addr, (uint8*) packet, ETH_TYPE_IP, length + sizeof(ip_datagram));

    kfree(packet);
}

void ip_recv_packet(ip_datagram *packet) {
    *((uint8*)(&packet->version_headerlength)) = ntoh8(*((uint8*)(&packet->version_headerlength)), 4);

    // Check for IPV4
    if (packet->version_headerlength & (4)) {

        //uint8 * data = ((uint8*) packet) + IP_HEADER_SIZE_32 * 4;

        switch(packet->protocol) {
            case IP_PROTOCOL_ICMP:
                printk(" ICMP TODO\n");
                break;
            case IP_PROTOCOL_UDP:
                break;
            default:
                printk(" unknown ip protocol\n");
        }

    } else {
        printk(" What the f is an IPv6?");
    }
}