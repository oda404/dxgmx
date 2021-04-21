
#include<dxgmx/string.h>

char *__strcat(char *__restrict dest, const char *__restrict src)
{
    size_t dest_i = __strlen(dest);
    size_t src_i = 0;

    for(; src_i < __strlen(src); ++src_i)
    {
        dest[dest_i++] = src[src_i];
    }
    dest[dest_i] = '\0';
    return dest;
}
