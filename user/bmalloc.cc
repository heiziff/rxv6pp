#include "user/bmalloc.h"
#include "user/user.h"

extern "C" {

block block_alloc(uint32_t size, uint32_t align) __attribute__((weak));

void block_free(block block) __attribute__((weak));
void setup_balloc(void) __attribute__((weak));
void setup_malloc() __attribute__((weak));

extern int bmalloc_enable_printing;
}


int bmalloc_enable_printing = 1;

block block_alloc(uint32_t size, uint32_t align) {
  if (bmalloc_enable_printing) printf("block_alloc default called\n");
  return {malloc(size), size, align};
}

void block_free(block block) {
  if (bmalloc_enable_printing) printf("block_free default called\n");
  free(block.begin);
}
void setup_balloc() {
  if (bmalloc_enable_printing) printf("setup_balloc default called\n");
}

void setup_malloc() {
  if (bmalloc_enable_printing) printf("setup_malloc default called\n");
}
