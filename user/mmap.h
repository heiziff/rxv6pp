#include "kernel/mmap.h"

#ifdef MAP_FAILED
#undef MAP_FAILED
#endif
#define MAP_FAILED ((void*)-1)
