/*! \file spinlock.h
 * \brief sleeplock structure
 * \remark lock that does not yield control
 */

#ifndef INCLUDED_kernel_spinlock_h
#define INCLUDED_kernel_spinlock_h

#ifdef __cplusplus
extern "C" {
#endif


// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};




#ifdef __cplusplus
}
#endif

#endif
