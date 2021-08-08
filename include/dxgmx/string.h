/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_STRING_H
#define _DXGMX_STRING_H

#include<stddef.h>
#include<dxgmx/attrs.h>

int strcmp(const char *str1, const char *str2)
_ATTR_PURE;

int memcmp(const void *str1, const void *str2, size_t n)
_ATTR_PURE;

size_t strlen(const char *str) 
_ATTR_PURE _ATTR_NONNULL(1);

char *strcat(char *__restrict dest, const char *__restrict src)
_ATTR_NONNULL(1, 2);

char *strncat(char *__restrict dest, const char *__restrict src, size_t n)
_ATTR_NONNULL(1, 2);

char *strcpy(char *__restrict dest, const char *__restrict src)
_ATTR_NONNULL(1, 2);

void *memset(void *__restrict s, int c, size_t n)
_ATTR_NONNULL(1);

void *memcpy(void *dest, const void *src, size_t n)
_ATTR_NONNULL(1, 2);

#endif // _DXGMX_STRING_H
