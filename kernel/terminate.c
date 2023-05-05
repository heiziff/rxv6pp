#include "defs.h"

uint64 sys_term() {
  timerhalt();
  return 0;
}

