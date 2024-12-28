#include <dlfcn.h>

#include "dynlink.h"

#ifndef __x86_64__

void *dlsym(void *restrict p, const char *restrict s) { return __dlsym(p, s, 0); }

#endif
