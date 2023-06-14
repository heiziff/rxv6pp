/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main() {
  int fd = open("tes", O_CREATE | O_RDWR);
  assert(fd > 0);

  char string[] = "Hallo mein Name ist Keko";
  assert(write(fd, string, strlen(string) + 1) == (int)strlen(string) + 1);

  close(fd);
  fd = open("tes", O_RDONLY);
  assert(fd > 0);

  void *va = mmap((void *)0, strlen(string) + 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  assert(va != (void*)MAP_FAILED);

  assert(strcmp((char *)va, string) == 0);

  assert(munmap(va, 4096) == 0);
  close(fd);

  char *string2 = (char *)malloc(2 * 4096 + 2);
  memset(string2, 'A', 2 * 4096 + 1);
  string2[2 * 4096 + 1] = 0;

  fd = open("tes2", O_CREATE | O_RDWR);
  assert(fd > 0);

  assert(write(fd, string2, strlen(string2) + 1) == (int)strlen(string2) + 1);

  close(fd);
  fd = open("tes2", O_RDONLY);
  assert(fd > 0);

  va = mmap((void *)0, strlen(string2) + 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  assert(va != (void*) MAP_FAILED);

  assert(strcmp((char *)va, string2) == 0);

  assert(munmap(va, 3 * 4096) == 0);
  close(fd);

  free(string2);

  char test_buf[] = "AAAABBBB";
  char result[]   = "AAAA";

  fd = open("tes3", O_CREATE | O_RDWR);
  assert(fd > 0);

  write(fd, test_buf, strlen(test_buf) + 1);
  close(fd);
  fd = open("tes3", O_RDONLY);

  va              = mmap((void *)0, 5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  assert(va != (void*)MAP_FAILED);
  ((char *)va)[4] = 0;
  assert(strcmp((char *)va, result) == 0);

  assert(munmap(va, 4096) == 0);
  close(fd);

  assert(!unlink("tes"));
  assert(!unlink("tes2"));
  assert(!unlink("tes3"));
}
