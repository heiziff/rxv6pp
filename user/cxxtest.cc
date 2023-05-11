#include "user/user.h"

const char string[] = "Hello World! ";

long long stringlen(char *buf) {
  if (!buf) return 0;
  long long i = 0;
  while (buf[i]) ++i;
  return i;
}

struct int_to_str_returnable {
  char buf[25];
};
struct int_to_str_returnable ll_to_string(long long value) {
  struct int_to_str_returnable wrapper = {0};

  for (int i = 0; value; ++i) {
    wrapper.buf[i] = (value % 10) + '0';
    value /= 10;
  }
  long long len = stringlen(wrapper.buf);
  for (int i = 0; i < stringlen(wrapper.buf) / 2; ++i) {
    char tmp                 = wrapper.buf[len - i - 1];
    wrapper.buf[len - i - 1] = wrapper.buf[i];
    wrapper.buf[i]           = tmp;
  }
  if (!wrapper.buf[0]) wrapper.buf[0] = '0';
  return wrapper;
}

template<typename T>
void print_size(T &&) {
  printf("size: %d\n", int(sizeof(T)));
}


using ll = long long;
int main(int argc, char **) {
  int group = 42;
  int copy  = group;

  struct int_to_str_returnable wrapper = ll_to_string(group);
  long long len                        = stringlen(wrapper.buf);
  wrapper.buf[len]                     = '\n';

  len += 1;
  if (sizeof(string) != write(1, string, sizeof(string))) return 1;
  if (len != write(1, wrapper.buf, len)) return 1;
  printf("%s%d\n", string, copy);

  //	if(argc > 1) hello(copy);
  print_size(int());
  print_size(ll());
  print_size(long());
  cxx(0);
  cxx(1);
  return 0;
}
