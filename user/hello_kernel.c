#include "kernel/types.h"
#include "user/user.h"

void main(int argc, char **argv) {
  if (argc < 2) {
    printf("Error: Not enough arguments\n");
    exit(-1);
  }
  int number = atoi(argv[1]);
  hello_kernel(number);
}