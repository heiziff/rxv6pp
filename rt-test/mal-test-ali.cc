/*!
 * \file
 * \brief test allocation alignments, using malloc
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void test_align(void *ptr, int alignment) {
  auto val = reinterpret_cast<uint64>(ptr);
  if (alignment & 1) return;
  if (alignment & 2) {
    assert((val & 1) == 0);
    return;
  }
  if (alignment & 4) {
    assert((val & 3) == 0);
    return;
  }
  if (alignment & 8) {
    assert((val & 7) == 0);
    return;
  }
  assert(!(val & 15));
}

void main() {
  setup_malloc();
  for (int i = 1; i < 16; ++i) {
    auto malloced = malloc(i);
    assert(malloced);
    test_align(malloced, i);
    free(malloced);
  }

  for (int i = 16; i < 4096; ++i) {
    auto malloced = malloc(i);
    assert(malloced);
    test_align(malloced, 16);
    free(malloced);
  }
}
