#ifndef _KERNEL_MMAP_H
#define _KERNEL_MMAP_H

#define PROT_NONE  (1 << 0)
#define PROT_READ  (1 << 1)
#define PROT_WRITE (1 << 2)
#define PROT_EXEC  (1 << 3)
#define PROT_RW (PROT_READ | PROT_WRITE)

#define MAP_ANON     (1 << 0)
#define MAP_PRIVATE  (1 << 1)
#define MAP_POPULATE (1 << 2)
#define MAP_FIXED    (1 << 3)

#endif // _KERNEL_MMAP_H
