/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_KSTDLIB_H__
#define __DXGMX_KSTDLIB_H__

#include<dxgmx/gcc/attrs.h>
#include<stddef.h>

void kfree(void *ptr);

void *kmalloc(size_t size)
__ATTR_MALLOC(free) __ATTR_ALLOC_SIZE(1);

long int __strtol(const char *__restrict str, char **__restrict endptr, int base);

int __abs(int n)
__ATTR_CONST;

long int __labs(long int n)
__ATTR_CONST;

long long int __llabs(long long int n)
__ATTR_CONST;

char *__itoa(int n, char *__restrict str, int base)
__ATTR_NONNULL(2);

void kabort(void)
__ATTR_NORETURN;

#endif //__DXGMX_KSTDLIB_H__
