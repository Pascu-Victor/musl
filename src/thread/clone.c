#include <errno.h>

#include "pthread_impl.h"

#ifndef __x86_64__

int __clone(int (*func)(void *), void *stack, int flags, void *arg, ...) { return -ENOSYS; }

#endif
