
#include "user/mmap.h"
#include "user/user.h"

extern "C" {
void *mmap(void *addr, uint64 length, int prot, int flags, int fd, uint64 offset) __attribute__((weak));
int munmap(void *addr, uint64 length) __attribute__((weak));
}

struct free_mapping {
  void *page_begin;
  void *free_begin;
};

void *round_up(void *ptr) {
  return reinterpret_cast<void *>((reinterpret_cast<uint64>(ptr) + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE);
}


constexpr auto memsize = PAGE_SIZE / sizeof(free_mapping);

static free_mapping memlist[memsize] = {};

void *mmap(void *addr, uint64 length, int prot, int flags, int fd, uint64 offset) {
  if (addr) return MAP_FAILED;
  if (length & (PAGE_SIZE - 1)) return MAP_FAILED;
  if (offset) return MAP_FAILED;
  if (prot != (PROT_READ | PROT_WRITE)) return MAP_FAILED;
  if ((flags != (MAP_PRIVATE | MAP_ANONYMOUS)) && (flags != (MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE)))
    return MAP_FAILED;
  if (!length) return MAP_FAILED;

  void *ptr = malloc(length + PAGE_SIZE);
  if (!ptr) return MAP_FAILED;
  void *page_ptr = round_up(ptr);
  for (unsigned i = 0; i < memsize; ++i) {
    if (!memlist[i].page_begin) {
      memlist[i].page_begin = page_ptr;
      memlist[i].free_begin = ptr;
      break;
    }
  }
  return page_ptr;
}

int munmap(void *addr, uint64 length) {
  if (!addr) return 0;
  if (reinterpret_cast<uint64>(addr) & (PAGE_SIZE - 1)) return 1;

  for (unsigned i = 0; i < memsize; ++i) {
    if (memlist[i].page_begin == addr) {
      free(memlist[i].free_begin);
      memlist[i] = free_mapping {};
      return 0;
    }
  }
  return 1;
}
