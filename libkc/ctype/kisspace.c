
#include<dxgmx/kctype.h>

int kisspace(int c)
{
    return 
    c ==  ' ' || 
    c == '\f' ||
    c == '\n' ||
    c == '\r' ||
    c == '\t' ||
    c == '\v';
}