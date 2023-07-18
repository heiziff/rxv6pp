#include "ip.h"
#include "ethernet.h"
#include "defs.h"

uint8 own_ip[4] = {10, 0, 2, 42};

uint16 ip_checksum(ip_datagram* header) {
    uint16 * it = (uint16*) header;

    int sum = 0;
    for(int i = 0; i < IP_HEADER_SIZE * 2; i++) {
        sum += ((1 << 16) - 1) - it[i];
    }

    return ((1 << 16) - 1) - sum;
}

uint8* get_ip(){
    return own_ip;
}

void ip_send_packet(uint8 *dst_ip, uint8 *data, uint8 protocol, uint32 length) {
    ip_datagram *packet = kalloc();
    memset(packet, 0, IP_HEADER_SIZE);

    packet->version_headerlength = (IP_HEADER_SIZE << 4) | 4;

    packet->length = hton16(IP_HEADER_SIZE + length);

    // identification only on fragment
    packet->ttl = 0xFF;

    packet->protocol = protocol;

    // TODO: reverse bytes?
    memcpy(packet->src_address, get_ip(), IPV4_ADDR_SIZE);

    memcpy(packet->dst_address, dst_ip, IPV4_ADDR_SIZE);


    // Lastly, compute checksum
    packet->checksum = ip_checksum(packet);







}