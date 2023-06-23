/*!
 * \file
 * \brief test that invalid m(un)map calls fail
 */

#include <rt-test/assert.h>
#include <user/user.h>
#include <kernel/fcntl.h>
#include <user/futex.h>

void main() {
  int fd = open("futex_tst.txt", O_CREATE | O_RDWR);
  assert(fd > 0);

  char string[32] = {'A'};
  memset(string, 'A', 32);
  assert(write(fd, string, 32) == 32);

  close(fd);
  fd = open("futex_tst.txt", O_CREATE | O_RDWR);

  printf("file war nett: fd %d\n", fd);

  void *va = mmap(0, 32, PROT_RW, MAP_SHARED, fd, 0);
  printf("va: \"%s\"\n", (char *)va);

  printf("mmap war maybe nett: %p\n", va);

  printf("osdev_mutex_t size: %d\n", sizeof(osdev_mutex_t));
  osdev_mutex_t *m = (osdev_mutex_t *)va;
  osdev_mutex_init(m);

  printf("osdev_mutex war nett\n");

  int pid = fork();
  if (pid > 0) // parent
  {
    osdev_mutex_lock(m);
    printf("parent %d got mutex (child %d)... ", getpid(), pid);
    int unused = 0;
    for (int i = 0; i < 100000000; i++) { unused += unused + i; }
    printf("irrelevant: %d\n", unused);
    printf("parent done\n");
    osdev_mutex_unlock(m);

  } else if (pid == 0) { // child
    osdev_mutex_lock(m);
    printf("child got mutex... ");
    int unused = 0;
    for (int i = 0; i < 100000000; i++) { unused += unused + i; }
    printf("irrelevant: %d\n", unused);
    printf("child done\n");
    osdev_mutex_unlock(m);
  } else {
    
    printf("dod weil fork\n");
    assert(0);
  }
}
