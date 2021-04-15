
#include<dxgmx/kstring.h>

size_t kstrlen(const char *str) 
{
    size_t ret = 0;
    while(str[ret] != '\0')
        ++ret;
    return ret;
}
