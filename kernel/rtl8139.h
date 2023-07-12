
#ifndef INCLUDED_RTL8139_H
#define INCLUDED_RTL8139_H

#ifdef __cplusplus
extern "C" {
#endif


#define RTL_MMIO_BASE 0x40000000
#define RTL_MMIO_SIZE 0x100
#define RTL_MEM_ADDR ((uint64)1 << 30)

enum RTL8139_registers {
  MAC0             = 0x00,       // Ethernet hardware address
  MAR0             = 0x08,       // Multicast filter
  TxStatus0        = 0x10,       // Transmit status (Four 32bit registers) (TSD)
  TxAddr0          = 0x20,       // Tx descriptors (also four 32bit) (TSAD)
  RxBuf            = 0x30,
  RxEarlyCnt       = 0x34,
  RxEarlyStatus    = 0x36,
  ChipCmd          = 0x37,
  RxBufPtr         = 0x38,        // 2 Byte?
  RxBufAddr        = 0x3A,
  IntrMask         = 0x3C,
  IntrStatus       = 0x3E,
  TxConfig         = 0x40,
  RxConfig         = 0x44,
  Timer            = 0x48,        // A general-purpose counter
  RxMissed         = 0x4C,        // 24 bits valid, write clears
  Cfg9346          = 0x50,
  Config0          = 0x51,
  Config1          = 0x52,
  FlashReg         = 0x54,
  GPPinData        = 0x58,
  GPPinDir         = 0x59,
  MII_SMI          = 0x5A,
  HltClk           = 0x5B,
  MultiIntr        = 0x5C,
  TxSummary        = 0x60,
  MII_BMCR         = 0x62,
  MII_BMSR         = 0x64,
  NWayAdvert       = 0x66,
  NWayLPAR         = 0x68,
  NWayExpansion    = 0x6A,

  // Undocumented registers, but required for proper operation
  FIFOTMS          = 0x70,        // FIFO Control and test
  CSCR             = 0x74,        // Chip Status and Configuration Register
  PARA78           = 0x78,
  PARA7c           = 0x7c,        // Magic transceiver parameter register
};

// Values for Interrupt mask register
enum RTL8139_int_bits {
  INT_ROK              = (1<<0),
  INT_RER              = (1<<1),
  INT_TOK              = (1<<2),
  INT_TER              = (1<<3),
};

enum RTL81319_tsd_bits {
  TSD_OWN             = (1 << 13),
  TSD_TOK             = (1 << 15),
};

//void* rtl8139_rx_buf;


bool_t rtl8139__init(void);

#ifdef __cplusplus
}
#endif

#endif
//INCLUDED_RTL8139_H
