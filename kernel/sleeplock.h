/*! \file sleeplock.h
 * \brief sleeplock structure
 * \remark lock that yields control
 */

#ifndef INCLUDED_kernel_sleeplock_h
#define INCLUDED_kernel_sleeplock_h

#ifdef __cplusplus
extern "C" {
#endif

#include "kernel/spinlock.h"

// Long-term locks for processes
struct sleeplock {
  uint locked;        // Is the lock held?
  struct spinlock lk; // spinlock protecting this sleep lock

  // For debugging:
  char *name; // Name of lock.
  int pid;    // Process holding lock
};




#ifdef __cplusplus
}
#endif

#endif
