#include "ethernet.h"
#include "rtl8139.h"
#include "defs.h"


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


void ethernet_send_packet(uint8 *dst_mac, uint8 *data, uint16 type, uint32 length) {
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

    switch(type){
        case ETH_TYPE_ARP:
            break;
        case ETH_TYPE_IP:
            break;
        default:
            panic("ethernet: unknown packet type");
            break;

    }

}