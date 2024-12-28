#include <math.h>

/* uses LLONG_MAX > 2^53, see comments in lrint.c */
#ifndef __x86_64__

long long llrint(double x) { return rint(x); }

#endif
