#include "defs.h"
#include "rtl8139.h"

uint8 mac_addr[6];

uint8 rx_buffer[8192 + 16];
uint32 rx_buffer_offset = 0;

// TODO: Maybe more work needs to be done on these descriptors, I don't fully understand them
//tx_descriptor tx_descr[4];
int cur_tx_descriptor = 0;
struct spinlock tx_descriptor_lock;

// static uint8 rtl_byte_r(uint8 reg) {
// 	return *((uint8*) RTL_MMIO_BASE + reg);
// }

static void rtl_byte_w(uint8 reg, uint8 val) {
	*((uint8*) ((uint64) RTL_MMIO_BASE + reg)) = val;
}

static uint16 rtl_word_r(uint8 reg) {
	return *((uint16*)((uint64) RTL_MMIO_BASE + reg));
}

static void rtl_word_w(uint8 reg, uint16 val) {
	*((uint16 *)((uint64) RTL_MMIO_BASE + reg)) = val;
}

static uint32 rtl_dword_r(uint8 reg) {
	return *( (uint32 *)((uint64) RTL_MMIO_BASE + reg));
}

static void rtl_dword_w(uint8 reg, uint32 val) {
	*((uint32 *)((uint64) RTL_MMIO_BASE + reg)) = val;
}

static uint32 rtl8139_rx_handler() {
  // TODO: evtl noch um die anderen Interrupts kümmern (siehe seite 9 Programmer manual)
  // writing bit clears interrupt
  rtl_word_w(IntrStatus, INT_ROK);
  printk(" rx_handler: call");

  rtl8139_recv_packet();

  return 0;
}

static uint32 rtl8139_tx_handler() {
  rtl_word_w(IntrStatus, INT_ROK);
  printk(" tx_handler: call");

  //uint8 desc = 0;
  //uint32 status = rtl_dword_r(TxStatus0 + (desc *4) & (TSD_OWN | TSD_TOK));
  // TODO: Check for finished descriptors and free them? Is this needed?????

  return 0;
}

uint32 rtl8139_intr() {
	// Check the reason for this interrupt
	printk(" GET INTERRUPTED!!!!!!!!!!!!");
	uint16 isr = rtl_word_r(IntrStatus);
	if (!(isr & INT_ROK) && !(isr & INT_TOK)) {
		panic("rtl8139: error interrupt");
	}

	if (isr & INT_ROK)
		return rtl8139_rx_handler();
	else if (isr & INT_TOK)
		return rtl8139_tx_handler();
	else {
		panic("rtl8139: how?");
		return 1;
	}
  return 0;
}

bool_t rtl8139__init()
{
  printk(" rtl_init: call\n");

  //int res = mappages(myproc()->pagetable, RTL_MEM_ADDR, 0x100, RTL_MMIO_BASE, PTE_R | PTE_W);
  //printk(" rtl_init: mapped");
  //if (res) panic("rtl_init: failed mapping");

  // Turn on rtl8139
  rtl_byte_w(Config1, 0x0);

  // Software reset to restore default values
  rtl_byte_w(ChipCmd, 0x10);

  // Spin waiting for device reset (or not :D )
  //while(rtl_byte_r(ChipCmd) & 0x10);

  // init receive buffer (and have fun with C)
  rtl_dword_w(RxBuf, *((uint32*) (&rx_buffer)));

  // Set interrupt mask register
  rtl_word_w(IntrMask, INT_ROK | INT_TOK);

  // Configure receive buffer (1 is wrap bit, f for enabled packets (we allow all because we want friends to talk to :) ))
  rtl_dword_w(RxConfig, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

  // Enable receive and transmit
  rtl_byte_w(ChipCmd, 0xc);


  // Save MAC Address
  uint32 mac_part1 = rtl_dword_r(MAC0);
  uint16 mac_part2 = rtl_dword_r(MAC0 + 4);
  mac_addr[0] = mac_part1 >> 0;
  mac_addr[1] = mac_part1 >> 8;
  mac_addr[2] = mac_part1 >> 16;
  mac_addr[3] = mac_part1 >> 24;

  mac_addr[4] = mac_part2 >> 0;
  mac_addr[5] = mac_part2 >> 8;

  return 0;
}

void rtl8139_get_mac(uint8 *buf) {
  memcpy(buf, mac_addr, 6);
}



void rtl8139_send_packet(void * data, uint32 len) {
    // First, copy the data to a physically contiguous chunk of memory
    // TODO: Check if len is larger than 1 page and think of something X)
    // FIXME: Oh god, this has to be 32 bit adress x)
    // TODO: Free this at some point? (maybe always use same transfer area? that might be bad with multiple requests)
    void * transfer_data = kalloc();
    memcpy(transfer_data, data, len);

    // Second, fill in physical address of data, and length
    acquire(&tx_descriptor_lock);

    rtl_dword_w(TxAddr0 + cur_tx_descriptor * 4, *((uint32*) &transfer_data));
    rtl_dword_w(TxStatus0 + cur_tx_descriptor * 4, len);
    cur_tx_descriptor = (cur_tx_descriptor + 1) % 4;

    release(&tx_descriptor_lock);
}

void rtl8139_recv_packet() {
  uint16 *packet_walker = (uint16*) (rx_buffer + rx_buffer_offset);

  // packet length at offset one
  uint16 packet_len = *(packet_walker + 1);
  (void) packet_len;
}