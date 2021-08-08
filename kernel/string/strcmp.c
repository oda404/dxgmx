/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/string.h>

int strcmp(const char *str1, const char *str2)
{
    size_t strlen1 = strlen(str1);
    size_t strlen2 = strlen(str2);

    if(strlen1 > strlen2)
        return 1;
    else if(strlen1 < strlen2)
        return -1;
    else
        return memcmp(str1, str2, strlen1);
}
