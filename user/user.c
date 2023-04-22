#include "kernel/syscall.h"
#include "kernel/types.h"
#include "user/user.h"

/**
 * Moves syscall number into a7 register, calls ecall
 */
#define SYSCALL(number) \
    asm volatile(       \
    "li a7, %0\n"       \
    "ecall\n"           \
    :                   \
    :"i"(number))

/**
 * Puts a system call return value from a0 register into a given variable.
 * All system calls return 64 Bit Values.
 */ 
#define SCGETRETURN(rVal) \
    asm volatile(             \
        "sw a0, %0\n"         \
        :"=m"(rVal)      \
    )

/**
 * Saves a0-a5 registers inside an array starting with aStart.
 * Only a0-a5 are supported for syscall arguments, as seen in argraw in syscall.c
*/
#define SCSAVEARGS(arr) \
    asm volatile(            \
        "sw a0, %0\n"        \
        "sw a1, %1\n"        \
        "sw a2, %2\n"        \
        "sw a3, %3\n"        \
        "sw a4, %4\n"        \
        "sw a5, %5\n"        \
        :"=m"(arr[0]),"=m"(arr[1]),"=m"(arr[2]),"=m"(arr[3]),"=m"(arr[4]),"=m"(arr[5])\
    )                        

/**
 * Restores registers a0 to a5 from inside an array.
*/
#define SCRESTOREARGS(arr) \
    asm volatile(            \
        "lw a0, %0\n"        \
        "lw a1, %1\n"        \
        "lw a2, %2\n"        \
        "lw a3, %3\n"        \
        "lw a4, %4\n"        \
        "lw a5, %5\n"        \
        :                    \
        :"m"(arr[0]),"m"(arr[1]),"m"(arr[2]),"m"(arr[3]),"m"(arr[4]),"m"(arr[5])\
    )   

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