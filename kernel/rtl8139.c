#include "defs.h"
#include "rtl8139.h"


uint8 RxBuffer[8192 + 16];

// static uint8 rtl_byte_r(uint8 reg) {
	// return *((uint8*) RTL_MMIO_BASE + reg);
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
  // writing bit clears interrupt
  rtl_word_w(IntrStatus, INT_ROK);
  printk(" rx_handler: call");

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
  rtl_dword_w(RxBuf, *((uint32*) (&RxBuffer)));

  // Set interrupt mask register
  rtl_word_w(IntrMask, INT_ROK | INT_TOK);

  // Configure receive buffer (1 is wrap bit, f for enabled packets (we allow all because we want friends to talk to :) ))
  rtl_dword_w(RxConfig, 0xf | (1 << 7)); // (1 << 7) is the WRAP bit, 0xf is AB+AM+APM+AAP

  // Enable receive and transmit
  rtl_byte_w(ChipCmd, 0xc);

  uint64 mac_addr = rtl_dword_r(MAC0);
  printk(" RTL_INIT: Got MAC: %p\n", mac_addr);

  // TODO: I think this should be 2 pages
  //rtl8139_rx_buf = kalloc();

  uint16 mask = rtl_word_r(IntrMask);
  printk(" RTL_INIT: Got IMR: %p\n", mask);

  return 0;
}

uint32 rtl8139_intr() {
	// Check the reason for this interrupt
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
