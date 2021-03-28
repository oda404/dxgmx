
#ifndef _STDLIB_H
#define _STDLIB_H

#include<stddef.h>
#include<dxgmx/gcc/attrs.h>

void free(void *ptr);
void *malloc(size_t size)
__ATTR_MALLOC(free) __ATTR_ALLOC_SIZE(1);

#endif // _STDLIB_H