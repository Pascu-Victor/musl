#include <math.h>

/* uses LONG_MAX > 2^24, see comments in lrint.c */
#ifndef __x86_64__

long lrintf(float x) { return rintf(x); }

#endif
