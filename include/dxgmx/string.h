
#ifndef __DXGMX_KSTRING_H__
#define __DXGMX_KSTRING_H__

#include<stddef.h>
#include<dxgmx/gcc/attrs.h>

int __memcmp(const void *str1, const void *str2, size_t n)
__ATTR_PURE;

size_t __strlen(const char *str) 
__ATTR_PURE __ATTR_NONNULL(1);

char *__strcat(char *__restrict dest, const char *__restrict src)
__ATTR_NONNULL(1, 2);

char *__strcpy(char *__restrict dest, const char *__restrict src)
__ATTR_NONNULL(1, 2);

#endif // __DXGMX_KSTRING_H__
