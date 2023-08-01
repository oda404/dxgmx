/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STRING_H
#define _DXGMX_STRING_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

int strcmp(const char* str1, const char* str2) _ATTR_PURE;

int memcmp(const void* str1, const void* str2, size_t n) _ATTR_PURE;

size_t strlen(const char* str) _ATTR_PURE _ATTR_NONNULL(1);

size_t strnlen(const char* str, size_t n) _ATTR_PURE _ATTR_NONNULL(1);

char* strcat(char* __restrict dest, const char* __restrict src)
    _ATTR_NONNULL(1, 2);

char* strncat(char* __restrict dest, const char* __restrict src, size_t n)
    _ATTR_NONNULL(1, 2);

char* strcpy(char* __restrict dest, const char* __restrict src)
    _ATTR_NONNULL(1, 2);

char* strncpy(char* __restrict dest, const char* __restrict src, size_t n)
    _ATTR_NONNULL(1, 2);

/* The pointer returned by this function needs to be free'd using kfree()
once it's no longer needed. */
_ATTR_NONNULL(1) char* strdup(const char* __restrict str);

void* memset(void* __restrict s, int c, size_t n) _ATTR_NONNULL(1);

void* memcpy(void* dest, const void* src, size_t n) _ATTR_NONNULL(1, 2);

char* strchr(const char* str, char c);
char* strrchr(const char* str, char c);

const char* strstr(const char* haystack, const char* needle);

#endif // _DXGMX_STRING_H
