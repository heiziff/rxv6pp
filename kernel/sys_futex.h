#ifndef _KERNEL_FUTEX_H
#define _KERNEL_FUTEX_H

#include <kernel/types.h>
#include <kernel/spinlock.h>

#define NFUTEX 32
#define FUTEX_INIT 100
#define FUTEX_WAIT 101
#define FUTEX_WAKE 102

typedef struct futex_queue_item_s {
  uint8 used;
  struct proc *proc;
  struct futex_queue_item_s *next;
  // not needed but temp for padding
  struct futex_queue_item_s *prev;
} futex_queue_item;

typedef struct futex_queue_s {
  void *page;
  futex_queue_item *first;
  futex_queue_item *last;
  uint64 size;
} futex_queue_t;

typedef struct kernel_futex_s {
  futex_queue_t waiting;
  uint64 pa; // physical addr of futex value in user space, needed to determine futex
  uint8 used;
  struct spinlock lock;
} kernel_futex_t;


#endif // _KERNEL_FUTEX_H
