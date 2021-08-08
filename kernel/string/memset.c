/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/string.h>
#include<stdint.h>

void *memset(void *s, int c, size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        ((uint8_t*)s)[i] = (uint8_t)c;
    }

    return s;
}

