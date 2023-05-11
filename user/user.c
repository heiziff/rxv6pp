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
        "sd a0, %0\n"        \
        :"=m"(rVal)          \
    )

/**
 * Saves a0-a5 registers inside an array.
 * Only a0-a5 are supported for syscall arguments, as seen in argraw in syscall.c, so only those are saved.
*/
#define SCSAVEARGS(arr)       \
    asm volatile(             \
        "sd a0, (0)%0\n"      \
        "sd a1, (8)%0\n"      \
        "sd a2, (16)%0\n"     \
        "sd a3, (24)%0\n"     \
        "sd a4, (32)%0\n"     \
        "sd a5, (40)%0\n"     \
        :"=o"(arr)            \
    )                        

/**
 * Restores registers a0 to a5 from inside an array.
*/
#define SCRESTOREARGS(arr)    \
    asm volatile(             \
        "ld a0, (0)%0\n"      \
        "ld a1, (8)%0\n"      \
        "ld a2, (16)%0\n"     \
        "ld a3, (24)%0\n"     \
        "ld a4, (32)%0\n"     \
        "ld a5, (40)%0\n"     \
        :                     \
        :"o"(arr)             \
    )   

void debugSCRegs(uint64 arr[6]) {
    for(int i = 0; i < 6; i++) {
        printf("a%d:%p\n",i,arr[i]);
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