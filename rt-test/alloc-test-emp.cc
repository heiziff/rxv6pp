/*!
 * \file
 * \brief test empty allocations
 */

#include "rt-test/assert.h"
#include "user/bmalloc.h"

void main() {
  setup_malloc();
  auto malloced = malloc(0);
  assert(!malloced);
  free(malloced);
}
