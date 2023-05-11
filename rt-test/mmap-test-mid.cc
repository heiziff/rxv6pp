/*!
 * \file
 * \brief munmap in the middle of the mapped area
 * \attention optional test case
 */


#include <rt-test/assert.h>
#include <user/mmap.h>

void main() {
  char *data = reinterpret_cast<char *>(
    mmap(0, 2 * PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  assert(!munmap(data + PAGE_SIZE, PAGE_SIZE));
  assert(!munmap(data, PAGE_SIZE));
}
