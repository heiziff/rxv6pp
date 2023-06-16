#include "memlayout.h"
#include "defs.h"

struct cpu cpus[NCPU];

typedef struct proc_queue_s {
  proc_queue_item *first;
  proc_queue_item *last;
  size_t size;
  proc_queue_item proc_q_itms[NPROC];
  struct spinlock lock;
} proc_queue;

typedef struct proc_queue_item_s {
  struct proc *proc;
  struct proc_queue_item_s *next;
} proc_queue_item;


struct proc procs[NPROC];

struct proc *initproc;

proc_queue proc_qs[NPRIO];

void proc_queue_init() 
{
  for (int prio = 0; prio < NPRIO; prio++) {

    initlock(proc_qs[prio].lock, "queue lock");
    for (int i = 0; i < NPROC; j++ ) {
      proc_qs[prio].proc_q_itms[i].next = proc_qs[prio].proc_q_itms[(i+1) % NPROC];
    }
    proc_qs[prio].first = proc_qs[prio].proc_q_itms[0];
    proc_qs[prio].last = proc_qs[prio].proc_q_itms[0];
    proc_qs[prio].size = 0;
  }
}

void enqueue_proc(proc_queue* p_q, struct proc *p)
{
  // TODO: What if procs are distributed between queues
  if (p_q->size == NPROC) {
    panic("Proc queue full");
    return;
  }

  p_q->last = p_q->last->next;
  p_q->last->proc = p;

  p_q->size++;
}

struct proc* dequeue_proc(proc_queue* p_q)
{
  if (p_q->size == 0) {
    panic("Proc queue empty");
    return;
  }

  struct proc *result = p_q->first->proc;

  p_q->first = p_q->first->next;

  p_q->size--;

  return result;
}


int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

// helps ensure that wakeups of wait()ing
// parents are not lost. helps obey the
// memory model when using p->parent.
// must be acquired before any p->lock.
struct spinlock wait_lock;

// Allocate a page for each process's kernel stack.
// Map it high in memory, followed by an invalid
// guard page.
void proc_mapstacks(pagetable_t kpgtbl) {
  struct proc *p;

  //TODO: Maybe do this lazy on process creation and not on startup
  for (p = procs; p < &procs[NPROC]; p++) {
    char *pa = kalloc();
    if (pa == 0) panic("kalloc");
    uint64 va = KSTACK((int)(p - procs));
    kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
  }
  
}

// initialize the proc table.
void procinit(void) {
  struct proc *p;

  initlock(&pid_lock, "nextpid");
  initlock(&wait_lock, "wait_lock");

  for (p = procs; p < &procs[NPROC]; p++) {
    initlock(&p->lock, "proc");
    p->state  = UNUSED;
    p->kstack = KSTACK((int)(p - procs));
  }
  
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int cpuid() {
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu *mycpu(void) {
  int id        = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc *myproc(void) {
  push_off();
  struct cpu *c  = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int allocpid() {
  int pid;

  acquire(&pid_lock);
  pid     = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc *allocproc(void) {
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid   = allocpid();
  p->state = USED;

  // Allocate a trapframe page.
  if ((p->trapframe = (struct trapframe *)kalloc()) == 0) {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if (p->pagetable == 0) {
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void freeproc(struct proc *p) {
  if (p->trapframe) kfree((void *)p->trapframe);
  // TODO: free mmapped_pages pls :)
  p->trapframe = 0;
  if (p->pagetable) proc_freepagetable(p->pagetable, p->sz, p->mmaped_pages);
  p->pagetable = 0;
  p->sz        = 0;
  p->pid       = 0;
  p->parent    = 0;
  p->name[0]   = 0;
  p->chan      = 0;
  p->killed    = 0;
  p->xstate    = 0;
  p->prio      = -1;
  p->state     = UNUSED;
}

// Create a user page table for a given process, with no user memory,
// but with trampoline and trapframe pages.
pagetable_t proc_pagetable(struct proc *p) {
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if (pagetable == 0) return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if (mappages(pagetable, TRAMPOLINE, PGSIZE, (uint64)trampoline, PTE_R | PTE_X) < 0) {
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe page just below the trampoline page, for
  // trampoline.S.
  if (mappages(pagetable, TRAPFRAME, PGSIZE, (uint64)(p->trapframe), PTE_R | PTE_W) < 0) {
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  p->mmaped_pages = kalloc();
  memset(p->mmaped_pages, 0, PGSIZE);

  return pagetable;
}

// we need to access the impl in proc_freepagetable
// unlucky C
int sys_munmap_impl(pagetable_t pagetable, taken_list *mmaped_pages, uint64 addr, int size);

// Free a process's page table, and free the
// physical memory it refers to.
void proc_freepagetable(pagetable_t pagetable, uint64 sz, taken_list *entry) {
  dbg(" PROC_FREEPGTBL: Call\n");

  taken_list *begin = entry;
  while (1) {
    //dbg(" PROC_FREEPGTBL: Spinning at %p\n", entry);
    if (entry->used) {
      dbg(" PROC_FREEPGTBL: found used entry at %p\n", entry->va);

      int ret = sys_munmap_impl(pagetable, begin, entry->va, entry->n_pages * PGSIZE);
      dbg(" PROC_FREEPGTBL: munmap ret %d\n", ret);
    }
    entry++;
    if ((uint64)entry % PGSIZE == 0) {
      if ((entry - 1)->next == 0) break;

      entry = (entry - 1)->next;
    }
  }

  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  dbg(" PROC_FREEPGTBL: Unmapped trampoline\n");
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  dbg(" PROC_FREEPGTBL: Unmapped trapframe\n");
  
  dbg(" PROC_FREEPGTBL: Done freeing mmapped\n");
  uvmfree(pagetable, sz);
  dbg(" PROC_FREEPGTBL: Done uvmfree\n");
}

// a user program that calls exec("/init")
// assembled from ../user/initcode.S
// od -t xC ../user/initcode
uchar initcode[] = {0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02, 0x97, 0x05, 0x00, 0x00, 0x93,
  0x85, 0x35, 0x02, 0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00, 0x93, 0x08, 0x20, 0x00, 0x73,
  0x00, 0x00, 0x00, 0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69, 0x74, 0x00, 0x00, 0x24, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Set up first user process.
void userinit(void) {
  struct proc *p;
  proc_queue_init();

  p        = allocproc();
  initproc = p;

  // allocate one user page and copy initcode's instructions
  // and data into it.
  uvmfirst(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;      // user program counter
  p->trapframe->sp  = PGSIZE; // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // should just go through since nobody should exist at this time
  acquire(&proc_qs[2].lock);
  p->state = RUNNABLE;
  enqueue_proc(&proc_qs[2], p);
  release(&proc_qs[2].lock);

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n) {
  uint64 sz;
  struct proc *p = myproc();

  sz = p->sz;
  if (n > 0) {
    if ((sz = uvmalloc(p->pagetable, sz, sz + n, PTE_W)) == 0) { return -1; }
  } else if (n < 0) {
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

taken_list* add_mapping(struct proc *p, uint64 va, int n_pages, int shared);

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int fork(void) {
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if ((np = allocproc()) == 0) { return -1; }

  // Copy user memory from parent to child.
  if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0) {
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  // TODO: Copy mmap memory from parent to child
  acquire(&p->lock);

  taken_list *l = p->mmaped_pages;
  while (1)
  {
    if (l->used) 
    {
      // copy taken_list entry from parent to child if its used
      taken_list *new_entry = add_mapping(np, l->va, l->n_pages, l->shared);
      dbg(" FORK: file %p, mapped_count %d\n", l->file, l->file->mapped_count);
      new_entry->file = l->file;
      l->file->mapped_count++;

      // add the actual mapping to the pagetable
      for (int i = 0; i < l->n_pages; i++) {
        pte_t *pte = walk(p->pagetable, l->va + i * PGSIZE, 0);
        uint64 pa = walkaddr(p->pagetable, l->va + i * PGSIZE);
        dbg(" FORK: mapping va %p to pa %p\n", new_entry->va, pa);
        mappages(np->pagetable, new_entry->va + i * PGSIZE, PGSIZE, pa, PTE_FLAGS(*pte));
      }
    }

    l++;
    if ((uint64)l % PGSIZE == 0) {
      if ((l-1)->next == 0) {
        break;
      };
      l = (l-1)->next;
    }
  }

  release(&p->lock);

  dbg(" FORK: Finished copying user memory\n");

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for (i = 0; i < NOFILE; i++)
    if (p->ofile[i]) np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);
  dbg(" FORK: Finished incrementing reference counters\n");

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  release(&np->lock);

  acquire(&wait_lock);
  np->parent = p;
  release(&wait_lock);

  acquire(&np->lock);
  np->state = RUNNABLE;
  np->prio = p->prio;

  acquire(&proc_qs[np->prio].lock);
  enqueue_proc(proc_qs[np->prio], np);
  release(&proc_qs[np->prio].lock);

  release(&np->lock);

  dbg(" FORK: Done!\n");

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold wait_lock.
void reparent(struct proc *p) {
  struct proc *pp;

  for (pp = proc; pp < &proc[NPROC]; pp++) {
    if (pp->parent == p) {
      pp->parent = initproc;
      wakeup(initproc);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void exit(int status) {
  struct proc *p = myproc();

  if (p == initproc) panic("init exiting");

  // Close all open files.
  for (int fd = 0; fd < NOFILE; fd++) {
    if (p->ofile[fd]) {
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  acquire(&wait_lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup(p->parent);

  acquire(&p->lock);

  p->xstate = status;
  p->state  = ZOMBIE;

  release(&wait_lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr) {
  struct proc *pp;
  int havekids, pid;
  struct proc *p = myproc();

  acquire(&wait_lock);

  for (;;) {
    // Scan through table looking for exited children.
    havekids = 0;
    for (pp = proc; pp < &proc[NPROC]; pp++) {
      if (pp->parent == p) {
        // make sure the child isn't still in exit() or swtch().
        acquire(&pp->lock);

        havekids = 1;
        if (pp->state == ZOMBIE) {
          // Found one.
          pid = pp->pid;
          if (addr != 0 && copyout(p->pagetable, addr, (char *)&pp->xstate, sizeof(pp->xstate)) < 0) {
            release(&pp->lock);
            release(&wait_lock);
            return -1;
          }
          freeproc(pp);
          release(&pp->lock);
          release(&wait_lock);
          return pid;
        }
        release(&pp->lock);
      }
    }

    // No point waiting if we don't have any children.
    if (!havekids || killed(p)) {
      release(&wait_lock);
      return -1;
    }

    // Wait for a child to exit.
    sleep(p, &wait_lock); //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void) {
  struct proc *p;
  struct cpu *c = mycpu();

  c->proc = 0;
  for (;;) {
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();

    for (int prio = 0; prio < NPRIO; prio++) {
      acquire(&proc_qs[prio].lock);
      if (proc_qs[prio].size == 0) continue;
      struct proc *p = dequeue_proc(proc_qs[prio]);
      acquire(&p->lock);
      if (p->state != RUNNABLE) panic("Unexpected un-runnable");
      p->state = RUNNING;
      c->proc = p;
      release(&proc_qs[prio].lock);
      swtch(&c->context, &p->context);


      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;

      acquire(&proc_qs[prio].lock);
      enqueue_proc(&proc_qs[prio], p);
      release(&proc_qs[prio].lock);
      
      release(&p->lock);
      break;
    }
    
  }
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
  int intena;
  struct proc *p = myproc();

  if (!holding(&p->lock)) panic("sched p->lock");
  if (mycpu()->noff != 1) panic("sched locks");
  if (p->state == RUNNING) panic("sched running");
  if (intr_get()) panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void) {
  struct proc *p = myproc();
  acquire(&p->lock);
  if(p->prio > 0) p->prio--;
  p->state = RUNNABLE;

  acquire(&proc_qs[p->prio]);
  enqueue_proc(proc_qs[p->prio], p);
  release(&proc_qs[p->prio]);

  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void) {
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (first) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void *chan, struct spinlock *lk) {
  struct proc *p = myproc();

  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.

  acquire(&p->lock); //DOC: sleeplock1
  release(lk);

  if (p->prio > 0) p->prio--;

  // Go to sleep.
  p->chan  = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  release(&p->lock);
  acquire(lk);
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void wakeup(void *chan) {
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++) {
    if (p != myproc()) {
      acquire(&p->lock);
      if (p->state == SLEEPING && p->chan == chan) { 

        acquire(&proc_qs[p->prio].lock);
        p->state = RUNNABLE; 
        enqueue_proc(proc_qs[p->prio], p);
        release(&proc_qs[p->prio].lock);
        }
      release(&p->lock);
    }
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int kill(int pid) {
  struct proc *p;

  for (p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if (p->pid == pid) {
      p->killed = 1;
      if (p->state == SLEEPING) {
        // Wake process from sleep().
        p->state = RUNNABLE;
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

void setkilled(struct proc *p) {
  acquire(&p->lock);
  p->killed = 1;
  release(&p->lock);
}

int killed(struct proc *p) {
  int k;

  acquire(&p->lock);
  k = p->killed;
  release(&p->lock);
  return k;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(int user_dst, uint64 dst, void *src, uint64 len) {
  struct proc *p = myproc();
  if (user_dst) {
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(void *dst, int user_src, uint64 src, uint64 len) {
  struct proc *p = myproc();
  if (user_src) {
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char *)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void procdump(void) {
  static char *states[] = {[UNUSED] "unused",
    [USED] "used",
    [SLEEPING] "sleep ",
    [RUNNABLE] "runble",
    [RUNNING] "run   ",
    [ZOMBIE] "zombie"};
  struct proc *p;
  char *state;

  pr_info("\n");
  for (p = proc; p < &proc[NPROC]; p++) {
    if (p->state == UNUSED) continue;
    if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    pr_info("%d %s %s", p->pid, state, p->name);
    pr_info("\n");
  }
}
