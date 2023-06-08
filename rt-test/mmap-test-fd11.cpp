/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main() {
  int fd = open("tes_t", O_CREATE | O_RDWR);
  assert(fd > 0);

  char string[6] = "AAAAA";

 // assert(write(fd, string, strlen(string) + 1) != (int)strlen(string) + 1);

  close(fd);
  fd = open("tes_t", O_RDONLY);

  char *va = (char*) mmap(0, strlen(string) + 1, PROT_RW, MAP_SHARED, fd, 0);
  memset(va, 'B', strlen(string));

  va[strlen(string)] = 0;

  printf(va);
  
}
