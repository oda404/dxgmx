/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/string.h>

#define UC unsigned char

int memcmp(const void *str1, const void *str2, size_t n)
{
    size_t i = 0;
    for(; i < n; ++i)
    {
        UC byte1 = *(UC *)(str1 + i);
        UC byte2 = *(UC *)(str2 + i);

        if(byte1 != byte2)
            return byte1 - byte2;
    }
    return 0;
}
