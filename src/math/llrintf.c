#include <math.h>

/* uses LLONG_MAX > 2^24, see comments in lrint.c */
#ifndef __x86_64__
long long llrintf(float x) { return rintf(x); }

#endif
