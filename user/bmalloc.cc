#include "user/bmalloc.h"
#include "user/user.h"

extern "C" {
  
block block_alloc(uint32_t size, uint32_t align) __attribute__((weak));

void block_free(block block) __attribute__((weak));
void setup_balloc(void) __attribute__((weak));
void setup_malloc() __attribute__((weak));

}


block block_alloc(uint32_t size, uint32_t align) {
  printf("block_alloc default called\n");
  return {malloc(size), size, align};
}

void block_free(block block) {
  printf("block_free default called\n");
  free(block.begin);
}
void setup_balloc() {
  printf("setup_balloc default called\n");
}

void setup_malloc() {
  printf("setup_malloc default called\n");
}
