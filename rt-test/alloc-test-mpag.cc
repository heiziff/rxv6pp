/*!
 * \file
 * \brief page aligned allocations
 * \attention optional test case
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void test_align(void *ptr, int alignment) {
	auto val = reinterpret_cast<uint64>(ptr);
	if (alignment & 1)
		return;
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
	assert((val & 15));
}

void main() {
	for (int i = 4096; i < 1 << 16; ++i) {
		void * ptr = malloc(i);
		assert(ptr);
		assert(!(reinterpret_cast<uint64>(ptr) & ((1 << 12) - 1)));
		free(ptr);

		auto block = block_alloc(i, 4096);
		assert(block.begin);
		assert(!(reinterpret_cast<uint64>(block.begin) & ((1 << 12) - 1)));
		block_free(block);
	}
}
