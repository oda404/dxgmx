/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/string.h>

void *memcpy(void *dest, const void *src, size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        ((char*)dest)[i] = ((const char*)src)[i];
    }
    return dest;
}
