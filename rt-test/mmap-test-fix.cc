/*!
 * \file
 * \brief test various MAP_FIXED versions
 * \attention optional test case
 */

#include "rt-test/assert.h"
#include "user/mmap.h"


void *const addr = reinterpret_cast<void *>(1 << 30);

void main() {
  void *ptr = mmap(addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  assert(ptr && ptr != MAP_FAILED);
  assert(ptr == addr);

  void *other =
    mmap(addr, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
  assert(other && other != MAP_FAILED);
  assert(other == addr);

  void *fail = mmap(addr, PAGE_SIZE, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
  assert(fail == MAP_FAILED);

  assert(!munmap(other, PAGE_SIZE));
}
