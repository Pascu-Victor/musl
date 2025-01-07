#include <stdlib.h>

#include "mallocng/glue.h"

weak void *realloc(void *p, size_t n) { return __libc_realloc(p, n); }
