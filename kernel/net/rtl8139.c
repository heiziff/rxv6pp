#include "../defs.h"
#include "../memlayout.h"
#include "rtl8139.h"


// static uint8 rtl_byte_r(uint8 reg) {
// 	return *(RTL_MMIO_BASE + reg);
// }

// static void rtl_byte_w(uint8 reg, uint8 val) {
// 	*(RTL_MMIO_BASE + reg) = val;
// }

// static uint16 rtl_word_r(uint8 reg) {
// 	return *( (uint16 *)(RTL_MMIO_BASE + reg));
// }

// static void rtl_word_w(uint8 reg, uint16 val) {
// 	*((uint16 *)(RTL_MMIO_BASE + reg)) = val;
// }
// static uint32 rtl_dword_r(uint8 reg) {
// 	return *( (uint32 *)(RTL_MMIO_BASE + reg));
// }

// static void rtl_dword_w(uint8 reg, uint32 val) {
// 	*((uint32 *)(RTL_MMIO_BASE + reg)) = val;
// }


bool_t rtl8139__init()
{
  printk(" rtl_init: call\n");

  int res = mappages(myproc()->pagetable, RTL_MEM_ADDR, 0x100, RTL_MMIO_BASE, PTE_R | PTE_W);
  printk(" rtl_init: mapped");
  if (res) panic("rtl_init: failed mapping");

  uint8 mac_addr = (uint8) *((uint8*)RTL_MMIO_BASE + MAC0);
  printk(" RTL_INIT: Got MAC: %p", mac_addr);

  // TODO: I think this should be 2 pages
  rtl8139_rx_buf = kalloc();

  return 0;
}

uint32 rtl8139_interrupt_handler(uint32 esp) {
	// Check the reason for this interrupt
	uint16 isr = rtl_word_r(IntrStatus);
	if (!(isr & ROK) && !(isr & TOK)) {
		panic("rtl8139: error interrupt");
	}

	if (isr & ROK)
		return rtl8139_rx_handler(esp);
	else if (isr & TOK)
		return rtl8139_tx_handler(esp);
	else {
		panic("Unhandled 8139 interrupt");
		return esp; // Silly, but gcc insists
	}
}
