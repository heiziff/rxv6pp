/*!
 * \file
 * \brief test allocation alignments
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"


void test_align(uint64 val, int alignment) {
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

void test_align(void *ptr, int alignment) {
  return test_align(reinterpret_cast<uint64>(ptr), alignment);
}

void main() {
  for (int i = 1; i < 16; ++i) {
    auto block = block_alloc(i, i & 1 ? 1 : i & 2 ? 2 : i & 4 ? 4 : 8);
    assert(block.begin);
    test_align(block.begin, i);
    test_align(block.align, i);
    assert(block.size >= static_cast<unsigned>(i));
    block_free(block);
  }

  for (int i = 16; i < 4096; ++i) {
    auto block1 = block_alloc(i, 16);
    assert(block1.begin);
    test_align(block1.begin, 16);
    test_align(block1.begin, 16);
    test_align(block1.align, 16);
    assert(block1.size >= static_cast<unsigned>(i));
    block_free(block1);

    auto block2 = block_alloc(i, 8);
    assert(block2.begin);
    test_align(block2.begin, 8);
    test_align(block2.begin, 8);
    test_align(block2.align, 8);
    assert(block2.size >= static_cast<unsigned>(i));
    block_free(block2);

    auto block3 = block_alloc(i, 4);
    assert(block3.begin);
    test_align(block3.begin, 4);
    test_align(block3.begin, 4);
    test_align(block3.align, 4);
    assert(block3.size >= static_cast<unsigned>(i));
    block_free(block3);

    auto block4 = block_alloc(i, 2);
    assert(block4.begin);
    test_align(block4.begin, 2);
    test_align(block4.begin, 2);
    test_align(block4.align, 2);
    assert(block4.size >= static_cast<unsigned>(i));
    block_free(block4);

    auto block5 = block_alloc(i, 1);
    assert(block5.begin);
    test_align(block5.begin, 1);
    test_align(block5.begin, 1);
    test_align(block5.align, 1);
    assert(block5.size >= static_cast<unsigned>(i));
    block_free(block5);
  }
}
