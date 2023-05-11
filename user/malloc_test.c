#include "bmalloc.h"
#include "user/user.h"

#define WEAK_TEST 0

#if WEAK_TEST

block block_alloc(uint32_t size, uint32_t align) {
  printf("block_alloc override called\n");
  return (block) {malloc(size), size, align};
}

void block_free(block block) {
  printf("block_free override called\n");
  free(block.begin);
}
void setup_balloc() {
  printf("setup_balloc override called\n");
}

void setup_malloc() {
  printf("setup_malloc override called\n");
}

#endif

void test_malloc(void) {
  setup_malloc();
  {
    int *foo = malloc(20);
    if (foo) {
      int sum = foo[0] + foo[1];
      (void)sum;
      memset(foo, 0, 20);
    }
    free(foo);
  }

  {
    void *foo = malloc(0);
    free(foo);
  }
}

void test_balloc(void) {
  setup_balloc();
  {
    block foo = block_alloc(20, _Alignof(int));
    if (foo.begin) {
      int *bar = foo.begin;
      int sum  = bar[0] + bar[1];
      (void)sum;
      memset(foo.begin, 0, 20);
    }
    block_free(foo);
  }
  {
    block foo = BALLOC(int, 5);
    if (foo.begin) {
      int *bar = foo.begin;
      int sum  = bar[0] + bar[1];
      (void)sum;
      memset(foo.begin, 0, 20);
    }
    block_free(foo);
  }
  {
    block foo = block_alloc(0, 1);
    block_free(foo);
  }
}

void main(int argc, char **argv) {
  printf("start alloc, used [%s]\n", (argc > 1) ? "malloc" : "block");
  if (argc > 1)
    test_malloc();
  else
    test_balloc();
}