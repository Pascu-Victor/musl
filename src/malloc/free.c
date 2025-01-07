#include <stdlib.h>

#include "mallocng/glue.h"

weak void free(void *p) { __libc_free(p); }
