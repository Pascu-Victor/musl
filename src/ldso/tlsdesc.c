#include <dynlink.h>
#include <stddef.h>

#ifndef __x86_64__

ptrdiff_t __tlsdesc_static() { return 0; }

weak_alias(__tlsdesc_static, __tlsdesc_dynamic);

#endif
