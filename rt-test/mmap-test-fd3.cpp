/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main() {
  int fd = open("hahahah.txt", O_CREATE | O_RDWR);
  assert(fd > 0);

  char string[] = "Halo ich mag mmap seeehr dolll";
  int size      = strlen(string) + 1;
  printf("strlen: %d\n", strlen(string) + 1);

  assert(write(fd, string, size) == size);

  close(fd);
  assert(!unlink("hahahah.txt"));
  // fd = open("mmap-fd11.txt", O_RDONLY);

  // char *va = (char*) mmap(0, strlen(string) + 1, PROT_RW, MAP_SHARED, fd, 0);
  // close(fd);
  // assert((uint64)va != MAP_FAILED);
  // memset(va, 'B', strlen(string));

  // va[strlen(string)] = 0;

  // printf(va);

  // if ((uint64)va != MAP_FAILED) munmap(va, strlen(string) + 1);
}
