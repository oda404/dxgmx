/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/string.h>

char *strcpy(char *dest, const char *src)
{
    size_t i = 0;
    size_t len = strlen(src);
    for(; i < len; ++i)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}
