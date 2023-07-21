#include "arp.h"
#include "rtl8139.h"
#include "ethernet.h"
#include "ip.h"
#include "defs.h"


uint8 ip[4] = {10, 0, 2, 42};

typedef struct arp_table_entry_s {
  uint8 ip_addr[4];
  uint8 mac_addr[6];
} arp_table_entry;

arp_table_entry arp_table[512];
uint16 arp_table_current_entry = 0;

uint8 *arp_lookup(uint8* ip_address) {
  for (int i = 0; i < 512; i++) {
    if (memcmp(ip_address, arp_table[i].ip_addr, 4)) {
      return arp_table[i].mac_addr;
    }
  }
}

void arp_send_packet(uint8 *target_haddr, uint8 *target_paddr) {
  printk(" arp_send: call\n");
  arp_packet *packet = (arp_packet *)kalloc();
  memset(packet, 0, 4096);

  packet->hardware_type = hton16(HARDWARE_TYPE_ETH);
  packet->protocol_type = hton16(PROTOCOL_TYPE_IPV4);

  packet->haddr_len = MAC_SIZE;
  packet->paddr_len = IPV4_ADDR_SIZE;

  // TODO: dont hardcode only arp requests
  packet->operation = hton16(ARP_OP_REQUEST);

  // Set src mac:
  rtl8139_get_mac(packet->sender_haddr);

  // Hardcode IP address (I hope this works)
  memcpy(packet->sender_paddr, ip, 4);

  // Fill in target mac
  memcpy(packet->target_haddr, target_haddr, MAC_SIZE);

  // Fill in target protocol addr
  memcpy(packet->target_paddr, target_paddr, IPV4_ADDR_SIZE);


  ethernet_send_packet(bcast_haddr, (uint8 *)packet, ETH_TYPE_ARP, sizeof(arp_packet));
}

void arp_receive_packet(arp_packet* packet, int len) {
  printk(" ARP_RECEIVE: call: src ip %d.%d.%d.%d\n", packet->sender_paddr[0], packet->sender_paddr[1], packet->sender_paddr[2], packet->sender_paddr[3]);
  char dst_hardware_addr[6];
  char dst_protocol_addr[4];
  // Save some packet field
  memcpy(dst_hardware_addr, packet->sender_haddr, 6);
  memcpy(dst_protocol_addr, packet->sender_paddr, 4);
  // Reply arp request, if the ip address matches(have to hard code the IP eveywhere, because I don't have dhcp yet)
  if (ntoh16(packet->operation) == ARP_OP_REQUEST) {
    if (memcmp(packet->target_paddr, ip, 4)) {
      printk(" ARP_RECEIVE: got request for our ip, answering with our mac\n");

      arp_send_packet((uint8*)dst_hardware_addr, (uint8*)dst_protocol_addr);

      printk(" ARP_RECEIVE: paket sent\n");
    }
  }
  else if(ntoh16(packet->operation) == ARP_OP_REPLY){
      //TODO: implement
      printk(" ARP_RECEIVE: received arp reply\n");
  }
  else {
      printk(" Got unknown ARP, opcode = %d\n", ntoh16(packet->operation));
  }

  // Now, store the ip-mac mapping
  memcpy(&arp_table[arp_table_current_entry].ip_addr, dst_protocol_addr, 4);
  memcpy(&arp_table[arp_table_current_entry].mac_addr, dst_hardware_addr, 6);

  if(arp_table_current_entry < 512)
      arp_table_current_entry++;
  // Wrap around
  if(arp_table_current_entry >= 512)
      arp_table_current_entry = 0;
}


uint64 sys_arp(void) {
    uint64 paddr;

    argaddr(0, &paddr);

    printk(" paddr is %p\n", paddr);
    uint8 target_paddr[4];

    copyin(myproc()->pagetable, (char*)target_paddr, paddr, IPV4_ADDR_SIZE);

    arp_send_packet(bcast_haddr, target_paddr);

    return 0;

}
