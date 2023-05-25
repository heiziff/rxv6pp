/*!
 * \file
 * \brief test empty allocations
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void main() {
  setup_balloc();
  auto block = block_alloc(0, 1);
  assert(!block.begin);
  block_free(block);
}
