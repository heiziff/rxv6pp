/*!
 * \file
 * \brief page aligned allocations
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void main() {
  for (int i = 4096; i < 1 << 16; ++i) {
    auto block = block_alloc(i, 4096);
    assert(block.begin);
    assert(!(reinterpret_cast<uint64>(block.begin) & ((1 << 12) - 1)));
    block_free(block);
  }
}
