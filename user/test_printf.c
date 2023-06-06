#include "kernel/types.h"
#include "user/user.h"

void main(int argc, char **argv) {
  printf("%d, %d, %d\n", 12, -12, 0xFFFFFFFFE);
  printf("%l, %l\n", 0xFFFFFFFFF, -0xFFFFFFFFF);
  printf("%x, %x\n", 0xFF, -0xFF);
  printf("%p\n", 0xFFFFFFFFF);
  printf("%s\n", "hi");
  printf("%u, %u\n", 12, -12);
  printf("%lu, %lu\n", 0xFFFFFFFFF, -0xFFFFFFFFF);
  printf("%lul\n", 0xFFF);
  printf("%b, %b, %b, %b, %b\n", 0b0100, 0b1000000, 0xFF0FF, 2, 0xFFFFFFFFF);
  printf("%l, %x, %b\n", -1, -1, -1);
}