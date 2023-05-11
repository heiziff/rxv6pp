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
    int *foo = static_cast<int *>(malloc(20));
    if (foo) {
      [[maybe_unused]] int sum = foo[0] + foo[1];
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
    auto foo = block_alloc_typed<int>(20 / sizeof(int));
    if (foo.begin) {
      [[maybe_unused]] int sum = foo.begin[0] + foo.begin[1];
      memset(foo.begin, 0, 20);
    }
    block_free(foo.untyped());
  }
  {
    block foo = BALLOC(int, 5);
    if (foo.begin) {
      int *bar = static_cast<int *>(foo.begin);
      int sum  = bar[0] + bar[1];
      (void)sum;
      memset(foo.begin, 0, 20);
    }
    block_free(foo);
  }
  {
    auto foo = block_alloc_typed<char>(0);
    block_free(foo.untyped());
  }
}

void main(int argc, char **argv) {
  printf("start alloc, used [%s]\n", (argc > 1) ? "malloc" : "block");
  if (argc > 1)
    test_malloc();
  else
    test_balloc();
}
