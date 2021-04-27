/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/string.h>

char *__strcpy(char *__restrict dest, const char *__restrict src)
{
    size_t i = 0;
    for(; i < __strlen(src); ++i)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}
