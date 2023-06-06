#include "user/user.h"

uint64 test(void* addr, int length, int prot, int flags) {
    printf("testing mmap at %p for size %d... ", addr, length);
    uint64 va = (uint64) mmap(addr, length, prot, flags);
    printf("got va %p\n", (void*)va);
    return va;
}

#define PGSIZE 4096

int main(int argc, char**) {
    test(0, 1, PROT_RW, MAP_PRIVATE);
    test((void*)PGSIZE, 1, PROT_RW, MAP_PRIVATE);
    test((void*)((uint64)1 << 35), 1, PROT_RW, MAP_PRIVATE);

    for (int i = 0; i < 3; i++) {
        test(0, 1, PROT_RW, MAP_PRIVATE);
    }
    for (int i = 0; i < 3; i++) {
        test((void*) ((uint64)1 << 36), 1, PROT_RW, MAP_PRIVATE);
    }
    for (int i = 0; i < 3; i++) {
        test((void*) ((uint64)PGSIZE), 1, PROT_RW, MAP_PRIVATE | MAP_FIXED);
    }
    uint64 addr = ((uint64) 1<<37);
    for (int i = 0; i < 5; i++) {
        test((void*) addr, i+1, PROT_RW, MAP_PRIVATE | MAP_FIXED);
        *(char*) (addr + i*PGSIZE) = 42;
        for (int j = 0; j <= i; j++) {
            if (*(char*) (addr + j*PGSIZE) != 42) {
                printf("oh oh, i: %d, j: %d, %d\n", i, j, *(char*) (addr + j*PGSIZE));
            }
        }
    }

    addr = ((uint64) 1<<34 | 1<<27);
    for (int i = 0; i < 5; i++) {
        test((void*) addr, i+1, PROT_RW, MAP_PRIVATE | MAP_FIXED);
    }
    for (int i = 0; i < 5; i++) {
        munmap((void*) addr, i+1);
    }
    printf("this should fail:\n");
    *(char*) addr = 42;

    return 0;
}

