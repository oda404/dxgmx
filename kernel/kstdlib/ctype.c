
#include<dxgmx/ctype.h>

int isdigit(int c)
{
    // basic ass ascii check
    return c >= '0' && c <= '9';
}

int isspace(int c)
{
    return 
    c ==  ' ' || 
    c == '\f' ||
    c == '\n' ||
    c == '\r' ||
    c == '\t' ||
    c == '\v';
}

int isxdigit(int c)
{
    if(
        (c >= '0' && c <= '9') || 
        (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F')
    )
        return 1;
    
    return 0;
}
