/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_KSTRING_H__
#define __DXGMX_KSTRING_H__

#include<stddef.h>
#include<dxgmx/attrs.h>

int __memcmp(const void *str1, const void *str2, size_t n)
_ATTR_PURE;

size_t __strlen(const char *str) 
_ATTR_PURE _ATTR_NONNULL(1);

char *__strcat(char *__restrict dest, const char *__restrict src)
_ATTR_NONNULL(1, 2);

char *__strcpy(char *__restrict dest, const char *__restrict src)
_ATTR_NONNULL(1, 2);

void *__memset(void *__restrict s, int c, size_t n)
_ATTR_NONNULL(1);

#endif // __DXGMX_KSTRING_H__
