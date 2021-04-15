
#include<dxgmx/kstring.h>

char *kstrcpy(char *__restrict dest, const char *__restrict src)
{
    size_t i = 0;
    for(; i < kstrlen(src); ++i)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}
