#include "sys_futex.h"
#include <kernel/defs.h>
#include <kernel/types.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define PGSIZE 4096
#define NFUTEX 32

futex_queue_item *futex_queue_find_space(futex_queue_t *q) {
  futex_queue_item *itm = (futex_queue_item *)q->page;
  uint64 max_size       = PGSIZE / sizeof(futex_queue_item);

  for (int i = 0; i < max_size; i++) {
    if (!itm[i].used) return (itm + i);
  }
  // no free queue items!
  panic("futex no queue space");
  return 0;
}

int futex_queue_enqueue(futex_queue_t *q, struct proc *proc) {
  futex_queue_item *itm = futex_queue_find_space(q);
  if (itm == 0) return -1;

  itm->used = 1;
  if (q->size == 0) {
    q->first = itm;
    q->last  = itm;
    q->size  = 1;
    return 0;
  }

  q->last->next = itm;
  q->last       = itm;
  q->size++;
  return 0;
}

struct proc *futex_queue_dequeue(futex_queue_t *q) {
  if (q->size == 0) {
    panic("futex empty dequeue");
    return 0;
  }

  futex_queue_item *itm = q->first;

  itm->used = 0;
  if (q->size == 0) q->first = q->first->next;
  q->size--;

  return itm->proc;
}

int futex_init_completed = 0;
kernel_futex_t kernel_futexes[NFUTEX];

void futex_init(kernel_futex_t *futex) {
  if (futex->waiting.page == 0) futex->waiting.page = kalloc();
  memset(futex->waiting.page, 0, PGSIZE);
  futex->used = 1;

  // futex_queue_t *t = futex->waiting;
  // while(!t->used) {
  //     t++;
  // }
}

kernel_futex_t *find_unused_futex() {
  for (int i = 0; i < NFUTEX; i++) {
    acquire(&kernel_futexes[i].lock);
    if (!kernel_futexes[i].used) {
      //! we are still holding the lock of the found futex!!
      return &kernel_futexes[i];
    }
    release(&kernel_futexes[i].lock);
  }
  return 0;
}

// Get the futex for a given addr. Returns with futex lock held
kernel_futex_t *find_futex_for_addr(uint64 pa) {
  for (int i = 0; i < NFUTEX; i++) {
    acquire(&kernel_futexes[i].lock);
    if (kernel_futexes[i].used && kernel_futexes[i].pa == pa) {
      //! we are still holding the lock of the found futex!!
      return &kernel_futexes[i];
    }
    release(&kernel_futexes[i].lock);
  }
  return 0;
}

// uint32_t *uaddr, int futex_op, uint32_t val, const struct timespec *timeout, /* or: uint32_t val2 */ uint32_t *uaddr2, uint32_t val3
uint64 sys_futex(void) {
  uint64 uaddr;
  int futex_op;
  int val;

  struct proc *p = myproc();

  argaddr(0, &uaddr);
  argint(1, &futex_op);
  argint(2, &val);

  if (!futex_init_completed) {
    printk(" futex init all\n");
    for (int i = 0; i < NFUTEX; i++) { initlock(&kernel_futexes[i].lock, "futex_lock"); }
    futex_init_completed = 1;
  }

  kernel_futex_t *futex = (void *)0;

  uint64 pa = walkaddr(p->pagetable, uaddr);
  if (pa == 0) goto bad;

  switch (futex_op) {
  case FUTEX_INIT:
    futex = find_unused_futex();
    if (!futex) panic("futex_init");
    futex_init(futex);
    release(&futex->lock);
    return 0;

  case FUTEX_WAIT:
    if (uaddr == val) {
      futex = find_futex_for_addr(pa);
      if (!futex) goto bad;
      futex_queue_enqueue(&futex->waiting, p);
      wait_on_futex(&futex->lock);
      release(&futex->lock);
      return 0;
      break;
    }
    goto bad;
    break;

  case FUTEX_WAKE:
    futex = find_futex_for_addr(pa);
    if (!futex) goto bad;
    uint32 wakeups = min(val, futex->waiting.size);
    for (int i = 0; i < wakeups; i++) {
      struct proc *p = futex_queue_dequeue(&futex->waiting);
      wakeup_on_futex(p);
    }
    release(&futex->lock);
    return wakeups;
    break;

  default: goto bad;
  }

  panic("unreachable");
  return 0;

bad:
  if (futex != (void *)0 && futex->used && holding(&futex->lock)) release(&futex->lock);
  return -1;
}
