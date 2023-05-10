#include "kernel/syscall.h"
#include "kernel/types.h"
#include "user/user.h"

/**
 * Moves syscall number into a7 register, calls ecall
 */
#define SYSCALL(number)      \
    asm volatile(            \
    "li a7, %0\n"            \
    "ecall\n"                \
    :                        \
    :"i"(number))

/**
 * Puts a system call return value from a0 register into a given variable.
 * All system calls return 64 Bit Values.
 */ 
#define SCGETRETURN(rVal)    \
    asm volatile(            \
        "sw a0, %0\n"        \
        :"=m"(rVal)          \
    )

/**
 * Saves a0-a5 registers inside an array.
 * Only a0-a5 are supported for syscall arguments, as seen in argraw in syscall.c, so only those are saved.
*/
#define SCSAVEARGS(arr)       \
    asm volatile(             \
        "sw a0, (0)%0\n"      \
        "sw a1, (8)%0\n"      \
        "sw a2, (16)%0\n"     \
        "sw a3, (24)%0\n"     \
        "sw a4, (32)%0\n"     \
        "sw a5, (40)%0\n"     \
        :"=o"(arr)            \
    )                        

/**
 * Restores registers a0 to a5 from inside an array.
*/
#define SCRESTOREARGS(arr)    \
    asm volatile(             \
        "lw a0, (0)%0\n"      \
        "lw a1, (8)%0\n"      \
        "lw a2, (16)%0\n"     \
        "lw a3, (24)%0\n"     \
        "lw a4, (32)%0\n"     \
        "lw a5, (40)%0\n"     \
        :                     \
        :"o"(arr)             \
    )   

void debugSCRegs(uint64 arr[6]) {
    for(int i = 0; i < 6; i++) {
        printf("a%d:%ul\n",i,arr[i]);
    }
}

/**
 * Handler for hello_kernel system call
*/
int hello_kernel(int group)
{
    uint64 registers[6] = {0};
    SCSAVEARGS(registers);
    /**
     * Do arg checking between SCSAVEARGS and SCRESTOREARGS
    */
    if (group > 33) {
        printf("There aren't that many groups, dummy\n");
        return -1;
    }
    
    SCRESTOREARGS(registers);
    SYSCALL(SYS_hello_kernel);
    uint64 rVal = 0;
    SCGETRETURN(rVal);
    return rVal;
} 