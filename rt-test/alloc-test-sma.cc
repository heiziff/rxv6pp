/*!
 * \file
 * \brief test small allocations
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"


void test_write(void *data, int bytes) {
  auto ptr = reinterpret_cast<char *>(data);
  for (int i = 0; i < bytes; ++i) ptr[i] = i + bytes;
  for (int i = 0; i < bytes; ++i) assert(ptr[i] == bytes + i);
}


void main() {
  setup_malloc();
  for (int i = 1; i <= 5; ++i) {
    auto bytes = 1 << i;

    auto malloced = malloc(bytes);
    assert(malloced);
    test_write(malloced, i);

    free(malloced);
  }
}
