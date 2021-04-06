
#include<string.h>

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

size_t strlen(const char *str) 
{
    size_t ret = 0;
    while(str[ret] != '\0')
        ++ret;
    return ret;
}

char *strcat(char *dest, const char *src)
{
    size_t dest_i = strlen(dest);
    size_t src_i = 0;

    for(; src_i < strlen(src); ++src_i)
    {
        dest[dest_i++] = src[src_i];
    }
    dest[dest_i] = '\0';
    return dest;
}

char *strcpy(char *dest, const char *src)
{
    size_t i = 0;
    for(; i < strlen(src); ++i)
    {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}
