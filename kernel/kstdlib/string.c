/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/string.h>
#include <dxgmx/types.h>

int memcmp(const void* str1, const void* str2, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        const u8 byte1 = ((const u8*)str1)[i];
        const u8 byte2 = ((const u8*)str2)[i];

        if (byte1 != byte2)
            return byte1 - byte2;
    }
    return 0;
}

void* memcpy(void* dest, const void* src, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        ((u8*)dest)[i] = ((const u8*)src)[i];
    return dest;
}

void* memset(void* s, int c, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        ((u8*)s)[i] = (u8)c;

    return s;
}

char* strcat(char* dest, const char* src)
{
    return strncat(dest, src, strlen(src));
}

char* strncat(char* dest, const char* src, size_t n)
{
    dest += strlen(dest);
    size_t srclen = strlen(src);
    size_t end = n < srclen ? n : srclen;

    for (size_t i = 0; i < end; ++i)
        *(dest++) = src[i];

    *dest = '\0';
    return dest;
}

char* strcpy(char* dest, const char* src)
{
    size_t i = 0;

    for (; src[i] != '\0'; ++i)
        dest[i] = src[i];

    dest[i] = '\0';

    return dest;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i = 0;

    for (; i < n && src[i] != '\0'; ++i)
        dest[i] = src[i];

    /* pad remaining bytes with '\0'. */
    for (; i < n; ++i)
        dest[i] = '\0';

    return dest;
}

int strcmp(const char* str1, const char* str2)
{
    size_t strlen1 = strlen(str1);
    size_t strlen2 = strlen(str2);

    if (strlen1 > strlen2)
        return 1;
    else if (strlen1 < strlen2)
        return -1;
    else
        return memcmp(str1, str2, strlen1);
}

size_t strlen(const char* str)
{
    size_t ret = 0;
    while (str[ret] != '\0')
        ++ret;
    return ret;
}

size_t strnlen(const char* str, size_t n)
{
    size_t ret = 0;
    while (str[ret] != '\0' && ret < n)
        ++ret;
    return ret;
}

char* strdup(const char* str)
{
    char* ret = kmalloc(strlen(str));
    if (ret)
        strcpy(ret, str);

    return ret;
}

char* strchr(const char* str, char c)
{
    const size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] == c)
            return (char*)&str[i];
    }

    return NULL;
}

char* strrchr(const char* str, char c)
{
    size_t len = minzu(strlen(str), _SSIZE_MAX_);
    // i = len because the null terminator is part of string as per the spec
    for (ssize_t i = len; i >= 0; --i)
    {
        if (str[i] == c)
            return (char*)&str[i];
    }

    return NULL;
}

const char* strstr(const char* haystack, const char* needle)
{
    size_t haylen = strlen(haystack);
    const size_t needlelen = strlen(needle);

    while (haylen >= needlelen)
    {
        if (memcmp(haystack, needle, needlelen) == 0)
            return haystack;

        ++haystack;
        --haylen;
    }

    return NULL;
}
