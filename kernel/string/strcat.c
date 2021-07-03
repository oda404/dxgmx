/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/string.h>

char *__strcat(char *__restrict dest, const char *__restrict src)
{
    return __strncat(dest, src, __strlen(src));
}
