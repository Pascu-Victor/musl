#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>

#include "syscall.h"

#ifndef __x86_64__

pid_t vfork(void) {
    /* vfork syscall cannot be made from C code */
#ifdef SYS_fork
    return syscall(SYS_fork);
#else
    return syscall(SYS_clone, SIGCHLD, 0);
#endif
}

#endif
