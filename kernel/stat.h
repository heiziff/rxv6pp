/*! \file stat.h
 * \brief struct the stat system call operates on
 */

#ifndef INCLUDED_kernel_stat_h
#define INCLUDED_kernel_stat_h

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel/types.h"


#define T_DIR 1    // Directory
#define T_FILE 2   // File
#define T_DEVICE 3 // Device

struct stat {
  int dev;     // File system's disk device
  uint ino;    // Inode number
  short type;  // Type of file
  short nlink; // Number of links to file
  uint64 size; // Size of file in bytes
};



#ifdef __cplusplus
}
#endif

#endif
