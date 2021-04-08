
#ifndef _STDLIB_H
#define _STDLIB_H

#include<stddef.h>
#include<dxgmx/gcc/attrs.h>

void free(void *ptr);
void *malloc(size_t size)
__ATTR_MALLOC(free) __ATTR_ALLOC_SIZE(1);
long int strtol(const char *__restrict str, char **__restrict endptr, int base);
int abs(int n);
long int labs(long int n);
long long int llabs(long long int n);
void abort(void) __ATTR_NORETURN;

#endif // _STDLIB_H