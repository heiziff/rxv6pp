/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>

void main() {
	int i = 0;

	int *ptr = &i;
	if(reinterpret_cast<uint64>(ptr) & (PAGE_SIZE - 1)) ++ptr;

	assert(mmap(ptr, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS) == (void*) MAP_FAILED);
	assert(mmap(0, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS) == (void*) MAP_FAILED);

	assert(munmap(ptr, 4096));
}
