
#include<dxgmx/kstring.h>

char *kstrcat(char *__restrict dest, const char *__restrict src)
{
    size_t dest_i = kstrlen(dest);
    size_t src_i = 0;

    for(; src_i < kstrlen(src); ++src_i)
    {
        dest[dest_i++] = src[src_i];
    }
    dest[dest_i] = '\0';
    return dest;
}
