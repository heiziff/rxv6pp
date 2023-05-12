/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/mmap.h>

void main() {
  int i = 0;

  assert(mmap(0, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) == MAP_FAILED);

  int *ptr = &i;
  if (reinterpret_cast<uint64>(ptr) & (PAGE_SIZE - 1)) ++ptr;
  assert(munmap(ptr, 4096));
}