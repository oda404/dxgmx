
#include<dxgmx/string.h>

char *__strncat(char *dest, const char *src, size_t n)
{
    dest += __strlen(dest);
    size_t srclen = __strlen(src);
    size_t end = n < srclen ? n : srclen;

    for(size_t src_i = 0; src_i < end; ++src_i)
        *(dest++) = src[src_i];

    *dest = '\0';
    return dest;
}

