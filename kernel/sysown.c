#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

uint64 sys_hello_kernel(void)
{
    int num;
    argint(0, &num);
    printf("Hello Kernelspace, Group %d\n", num);
    return 2;
}