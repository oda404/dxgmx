/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/string.h>

char *strcat(char *dest, const char *src)
{
    return strncat(dest, src, strlen(src));
}
