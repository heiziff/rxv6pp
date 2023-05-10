#include "user/user.h"

uint64 test(void* addr, int length, int prot, int flags) {
    printf("testing mmap at %p for size %d... ", addr, length);
    uint64 va = (uint64) mmap(addr, length, prot, flags);
    printf("got va %p\n", (void*)va);
    return va;
}

int main(int argc, char**) {
    test(0, 1, PROT_RW, MAP_PRIVATE);
    test((void*)0x100, 1, PROT_RW, MAP_PRIVATE);
    test((void*)((uint64)1 << 35), 1, PROT_RW, MAP_PRIVATE);

    for (int i = 0; i < 3; i++) {
        test(0, 1, PROT_RW, MAP_PRIVATE);
    }
    for (int i = 0; i < 3; i++) {
        test((void*) ((uint64)1 << 36), 1, PROT_RW, MAP_PRIVATE);
    }

    return 0;
}

