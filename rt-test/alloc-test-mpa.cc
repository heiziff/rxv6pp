/*!
 * \file
 * \brief page aligned allocations
 * \attention optional test case
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void main() {
  for (int i = 4096; i < 1 << 16; ++i) {
    void *ptr = malloc(i);
    assert(ptr);
    assert(!(reinterpret_cast<uint64>(ptr) & ((1 << 12) - 1)));
    free(ptr);
  }
}
