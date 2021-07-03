/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_KSTDLIB_H__
#define __DXGMX_KSTDLIB_H__

#include<dxgmx/attrs.h>
#include<stddef.h>

void kfree(void *ptr);

void *kmalloc(size_t size)
_ATTR_MALLOC(free) _ATTR_ALLOC_SIZE(1);

long int __strtol(const char *__restrict str, char **__restrict endptr, int base);

int __abs(int n)
_ATTR_CONST;

long int __labs(long int n)
_ATTR_CONST;

long long int __llabs(long long int n)
_ATTR_CONST;

char *__itoa(int n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *__ltoa(long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *__lltoa(long long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *__utoa(unsigned n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *__ultoa(unsigned long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *__ulltoa(unsigned long long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

void kabort(void)
_ATTR_NORETURN;

#endif //__DXGMX_KSTDLIB_H__
