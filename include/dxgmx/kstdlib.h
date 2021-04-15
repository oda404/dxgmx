
#ifndef __DXGMX_KSTDLIB_H__
#define __DXGMX_KSTDLIB_H__

#include<dxgmx/gcc/attrs.h>
#include<stddef.h>

void kfree(void *ptr);

void *kmalloc(size_t size)
__ATTR_MALLOC(free) __ATTR_ALLOC_SIZE(1);

long int kstrtol(const char *__restrict str, char **__restrict endptr, int base);

int kabs(int n);

long int klabs(long int n);

long long int kllabs(long long int n);

char *kitoa(int n, char *__restrict str, int base)
__ATTR_NONNULL(2);

void kabort(void)
__ATTR_NORETURN;

#endif //__DXGMX_KSTDLIB_H__
