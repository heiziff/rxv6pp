#include "ethernet.h"
#include "rtl8139.h"
#include "defs.h"
#include "arp.h"
#include "ip.h"


// Flip byte ordering to network byte order 16 bit (htons)
uint16 hton16(uint16 h_val) {
    uint16 lower = (h_val & 0xFF) << 8;
    uint16 higher = (h_val >> 8) & 0xFF;
    uint16 net_val = lower | higher;

    return net_val;
}

// Flip byte ordering to host byte order 16 bit (ntohs)
uint16 ntoh16(uint16 n_val) {
    return hton16(n_val);
}

uint8 hton8(uint8 byte, int num_bits) {
    uint8 res = 0;
    res = byte << (8 - num_bits);
    return res | (byte >> num_bits);
}

uint8 ntoh8(uint8 byte, int num_bits) {
    return hton8(byte, num_bits);
}

uint32 hton32(uint32 h_val) {
    return ((h_val & 0x000000FF) << 24) | ((h_val & 0x0000FF00) << 8) |
           ((h_val & 0x00FF0000) >> 8) | ((h_val & 0xFF000000) >> 24);
}

// uint32 ntoh32(uint32 n_val) {
//     return hton32(n_val);
// }


void ethernet_send_packet(uint8 *dst_mac, uint8 *data, uint16 type, uint32 length) {
    printk(" ethernet_send: call type 0x%x, length %d\n", type, length);

    // TODO: Ethernet frames are always page size, which is kinda wasteful
    ethernet_frame *frame = (ethernet_frame*) kalloc();
    uint8 *frame_data = ((uint8*) frame) + sizeof(ethernet_frame);

    // Fill in src MAC and dst MAC
    rtl8139_get_mac(frame->src_mac);
    //memcpy(frame->src_mac, src_mac, MAC_SIZE);
    memcpy(frame->dst_mac, dst_mac, MAC_SIZE);

    frame->type = hton16(type);

    memcpy(frame_data, data, length);

    rtl8139_send_packet(frame, sizeof(ethernet_frame) + length);

    kfree(frame);
}


void ethernet_recv_packet(ethernet_frame *frame, uint32 length) {
    uint16 type = ntoh16(frame->type);
    printk(" ETHERNET_RECV: call\n");

    switch(type){
        case ETH_TYPE_ARP:
            printk(" ETHERNET_RECV: call arp\n");
            arp_receive_packet((void*)frame + sizeof(ethernet_frame), length);
            break;
        case ETH_TYPE_IP:
            printk(" ETHERNET_RECV: ip\n");
            ip_recv_packet((void*)frame + sizeof(ethernet_frame));
            break;
        default:
            panic("ethernet: unknown packet type");
            break;

    }

}
