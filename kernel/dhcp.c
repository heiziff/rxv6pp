#include "dhcp.h"
#include "defs.h"
#include "udp.h"
#include "ip.h"
#include "arp.h"
#include "ethernet.h"
#include "rtl8139.h"

void dhcp_send_discover() {
    dhcp_packet *packet = kalloc();
    memset(packet, 0, sizeof(dhcp_packet));

    packet->operation = DHCP_CLIENT;
    packet->hw_type = HARDWARE_TYPE_ETH;
    packet->hw_length = MAC_SIZE;
    packet->hops = 0;

    packet->transaction_id = hton32(XID);

    packet->secs = 0;
    packet->flags = 0;

    rtl8139_get_mac(packet->client_haddr);

    packet->magic_cookie = hton32(DHCP_MAGIC);

    uint8* options = packet->options;

    // Set option for DHCP operation at 53 
    *(options++) = 53;
    *(options++) = 1;
    *(options++) = DHCP_DISCOVER;

    // Set requested IP address at 50
    *(options++) = 50;
    *(options++) = IPV4_ADDR_SIZE;
    *((uint32*) (options)) = hton32(0x0a00022A);
    options += IPV4_ADDR_SIZE;

    // Set hostname
    *(options++) = 12;
    *(options++) = (strlen("rxv6pp") + 1);
    memcpy(options, "rxv6pp", strlen("rxv6pp") + 1);
    options += strlen("rxv6pp") + 1;


    // Parameter request list
    *(options++) = 55;
    *(options++) = 4;
    *(options++) = 0x1;
    *(options++) = 0x3;
    *(options++) = 0x6;
    *(options++) = 0xf;
   
    // Set end marker
    *(options++) = 0xff;


    uint8 dst_ip[IPV4_ADDR_SIZE];
    memset(dst_ip, 0xFF, IPV4_ADDR_SIZE);

    // Done building, send packet:
    udp_send_packet(dst_ip, DHCP_PORT_CLIENT, DHCP_PORT_SERVER, (uint8*) packet, sizeof(dhcp_packet));
    kfree(packet);
}

void dhcp_send_request(uint8 *requested_ip) {
    dhcp_packet *packet = kalloc();
    memset(packet, 0, sizeof(dhcp_packet));

    // Wikipedia says so, but why???
    packet->operation = DHCP_CLIENT;
    packet->hw_type = HARDWARE_TYPE_ETH;
    packet->hw_length = MAC_SIZE;
    packet->hops = 0;

    packet->transaction_id = hton32(XID);

    // TODO: I think we have to specify DHCP server IP

    rtl8139_get_mac(packet->client_haddr);

    packet->magic_cookie = hton32(DHCP_MAGIC);

    uint8* options = packet->options;

    // Operation mode
    *(options++) = 53;
    *(options++) = 1;
    *(options++) = DHCP_REQUEST;

    // Requested IP
    *(options++) = 50;
    *(options++) = IPV4_ADDR_SIZE;
    memcpy(options, requested_ip, IPV4_ADDR_SIZE);
    options += IPV4_ADDR_SIZE;

    // Set hostname
    *(options++) = 12;
    *(options++) = (strlen("rxv6pp") + 1);
    memcpy(options, "rxv6pp", strlen("rxv6pp") + 1);
    options += strlen("rxv6pp") + 1;

    // Set end marker
    *(options++) = 0xff;

    uint8 dst_ip[IPV4_ADDR_SIZE];
    memset(dst_ip, 0xFF, IPV4_ADDR_SIZE);

    // Done building, send packet:
    udp_send_packet(dst_ip, DHCP_PORT_CLIENT, DHCP_PORT_SERVER, (uint8*) packet, sizeof(dhcp_packet));
    kfree(packet);
}

void dhcp_recv_packet(dhcp_packet *packet) {
    uint8 *option = packet->options;

    uint8 packet_type = 0;
    if (packet->operation == DHCP_SERVER) {
        // Check if this is Offer or ACK packet
        while(*option != 0xFF) {
            // Skip magic (is this in options or not????)
            if (*((uint32*) option) == DHCP_MAGIC) option+=4;

            if (*option == 53) {
                packet_type = option[2];
                break;
            }
            uint8 option_len = option[1];
            option += (option_len + 1);
        }

        switch(packet_type) {
            case(DHCP_OFFER):
                dhcp_send_request(packet->your_ip);
                break;
            case(DHCP_ACK):
                // Save our IP
                memcpy(own_ip, packet->your_ip, IPV4_ADDR_SIZE);
                break;
            default:
                printk(" Unknown DHCP packet received\n");
        }

    }
    else {
        printk(" Got unexpected dhcp client packet, ignoring\n");
    }
}