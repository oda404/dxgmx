/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/string.h>

int memcmp(const void *str1, const void *str2, size_t n)
{
    for(size_t i = 0; i < n; ++i)
    {
        char byte1 = ((const char *)str1)[i];
        char byte2 = ((const char *)str2)[i];

        if(byte1 != byte2)
            return byte1 - byte2;
    }
    return 0;
}
