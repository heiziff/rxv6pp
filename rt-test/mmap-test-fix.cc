/*!
 * \file
 * \brief test various MAP_FIXED versions
 * \attention optional test case
 */

#include "rt-test/assert.h"
#include "user/user.h"


void *const addr = reinterpret_cast<void*>((uint64)1 << 32);

void main() {
    void *ptr = mmap(addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED);
    assert(ptr && ptr != (void*) MAP_FAILED);
    assert(ptr == addr);

    void *other = mmap(addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED);
    assert(other && other != (void*) MAP_FAILED);
    assert(other == addr);

    void *fail = mmap(addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE);
    assert(fail == (void*) MAP_FAILED);

    assert(!munmap(other, PAGE_SIZE));
}
