#include <math.h>
#include <stdint.h>

#ifndef __x86_64__

float fabsf(float x) {
    union {
        float f;
        uint32_t i;
    } u = {x};
    u.i &= 0x7fffffff;
    return u.f;
}

#endif
