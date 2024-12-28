#include <stdlib.h>

#include "mallocng/glue.h"

void free(void *p) { __libc_free(p); }
