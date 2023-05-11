#include "defs.h"

template<int What>
void a_template() {
  printf(const_cast<char *>("cpp printed: %d\n"), What);
}
extern "C" uint64 sys_cxx() {
  int argnr;
  argint(0, &argnr);
  if (argnr & 1)
    a_template<1>();
  else
    a_template<0>();
  return 0;
}