/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>

void main() {
  int fd = open("mmap-fd.txt", O_CREATE | O_RDWR);
  assert(fd > 0);

  char string[]  = "Halo ich mag mmap seeehr dolll";
  char string2[] = "Halo ich mag mmBBBBBBBBBBBBBBB";
  char string3[] = "AAAAAAAAAAAAAAABBBBBBBBBBBBBBB";
  int size       = strlen(string) + 1;
  assert(write(fd, string, size) == size);

  close(fd);

  fd = open("mmap-fd.txt", O_RDONLY);

  int pid = fork();
  if (pid > 0) // parent
  {
    printf("starting parent!\n");
    void *va = mmap(0, size, PROT_RW, MAP_SHARED, fd, 0);
    assert((uint64)va != MAP_FAILED);
    int keine_ahnung = 0;
    if (wait(&keine_ahnung) < 0) {
      printf("HÄ?\n");
      assert(0);
      return;
    }

    printf("mapped %p\n", va);
    printf("str: \"%s\"\n", (char *)va);
    assert(strcmp((char *)va, string2) == 0);
    memset(va, 'A', 15);
    assert(strcmp((char *)va, string3) == 0);
    close(fd);

    assert(munmap(va, 4096) == 0);

    assert(!unlink("mmap-fd.txt"));

  } else if (pid == 0) // child
  {
    for (int i = 0; i < 1000000; i++)
      ;
    void *va = mmap(0, size, PROT_RW, MAP_SHARED, fd, 0);
    assert((uint64)va != MAP_FAILED);
    printf("child: %p\n", va);
    assert(strcmp((char *)va, string) == 0);

    memset((char *)va + 15, 'B', 15);
    assert(strcmp((char *)va, string2) == 0);
    close(fd);
    printf("child done\n");
    assert(munmap(va, 4096) == 0);
  } else {
    printf("dod weil fork\n");
    assert(0);
  }
}
