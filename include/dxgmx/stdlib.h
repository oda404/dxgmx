/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_STDLIB_H
#define _DXGMX_STDLIB_H

#include<dxgmx/attrs.h>
#include<stddef.h>

void kfree(void *ptr);

void *kmalloc(size_t size)
_ATTR_MALLOC(free) _ATTR_ALLOC_SIZE(1);

long int strtol(const char *__restrict str, char **__restrict endptr, int base);

int abs(int n)
_ATTR_CONST;

long int labs(long int n)
_ATTR_CONST;

long long int llabs(long long int n)
_ATTR_CONST;

char *itoa(int n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *ltoa(long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *lltoa(long long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *utoa(unsigned n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *ultoa(unsigned long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

char *ulltoa(unsigned long long n, char *__restrict str, int base)
_ATTR_NONNULL(2);

#endif //_DXGMX_STDLIB_H
