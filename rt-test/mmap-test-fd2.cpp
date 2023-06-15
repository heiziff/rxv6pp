/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main() {
  printf("start\n");
  int fd = open("mmap-fd2.txt", O_CREATE | O_RDWR);
  assert(fd > 0);

  char string[]  = "Halo ich mag mmap seeehr dolll";
  char string2[] = "Halo ich mag mmBBBBBBBBBBBBBBB";
  char string3[] = "AAAAAAAAAAAAAAABBBBBBBBBBBBBBB";
  int size       = strlen(string) + 1;
  if (write(fd, string, size) != size) {
    printf("write failed\n");
    return;
  }
  printf("wrote\n");

  close(fd);
  printf("closing initial fd\n");

  fd       = open("mmap-fd2.txt", O_RDONLY);
  assert(fd > 0);
  void *va = mmap(0, size, PROT_RW, MAP_SHARED, fd, 0);
  close(fd);

  printf("Forked!\n");
  int pid = fork();
  if (pid > 0) // parent
  {
    int keine_ahnung;
    if (wait(&keine_ahnung) < 0) {
      printf("HÄ?\n");
      assert(0);
      return;
    }

    assert(strcmp((char *)va, string2) == 0);
    memset(va, 'A', 15);
    assert(strcmp((char *)va, string3) == 0);

    assert(!unlink("mmap-fd2.txt"));
    assert(munmap(va, 4096) == 0);
    printf("exiting parent\n");

  } else if (pid == 0) // child
  {
    printf("Starting child\n");
    assert(strcmp((char *)va, string) == 0);
    printf("Memsetting Bs\n");

    memset((char *)va + 15, 'B', 15);
    assert(strcmp((char *)va, string2) == 0);
    assert(munmap(va, 4096) == 0);
    exit(0);
  } else {
    printf("dod weil fork\n");
    assert(0);
  }
}
