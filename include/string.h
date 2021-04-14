
#ifndef _STRING_H
#define _STRING_H

#include<stddef.h>
#include<dxgmx/gcc/attrs.h>

int memcmp(const void *str1, const void *str2, size_t n)
__ATTR_PURE;

size_t strlen(const char *str) 
__ATTR_PURE __ATTR_NONNULL(1);

char *strcat(char *__restrict dest, const char *__restrict src)
__ATTR_NONNULL(1, 2);

char *strcpy(char *__restrict dest, const char *__restrict src)
__ATTR_NONNULL(1, 2);

#endif // _STRING_H
