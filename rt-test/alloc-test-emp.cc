/*!
 * \file
 * \brief test empty allocations
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void main() {
	auto malloced = malloc(0);
	assert(!malloced);
	auto block = block_alloc(0, 1);
	assert(!block.begin);
	free(malloced);
	block_free(block);
}
