// init: The initial user-level program

#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

const char *argv[] = { "sh", 0 };

bool substring(const char *pattern, const char *string) {
  int pattern_index = 0;
  int index = 0;
  while (string[index]) {
    if (!pattern[pattern_index]) return true;
    if (string[index] == pattern[pattern_index]) {
      ++pattern_index;
    }
    else
      pattern_index = 0;
    ++index;
  }
  return !pattern[pattern_index];
}

int main(void) {
  if(open("console", O_RDWR) < 0){
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  // look up testcases
  int fd = open("/", O_RDONLY);

  for (dirent entry; read(fd, &entry, sizeof(dirent)) == sizeof(dirent); ) {
    if(!entry.name[0]) continue;
    if (substring("test", entry.name)) {
      printf(">>>> starting test [%s]\n", entry.name);
      auto pid  = fork();
      if(pid < 0) exit(1);
      else if (pid == 0) {
        exec(entry.name, const_cast<char**>(argv));
        printf(">>>> exit failed\n");
        exit(1);
      }
      int retcode = 0;
      auto ret = wait(&retcode);
      if (ret != pid) exit(1); // we're not really init
      if (retcode)
        printf(">>>> testcase [%s] failed with %d\n", entry.name, retcode);
      else
        printf(">>>> testcase [%s] succeeded\n", entry.name, retcode);
    }
  }

  // leads to a crash, but who really cares?
  printf(">>>> finished tests, crashing now\n");
  exit(0);
}
