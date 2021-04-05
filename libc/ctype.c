
#include<ctype.h>

int isdigit(int c)
{
    // basic ass ascii check
    return c >= '0' && c <= '9';
}

int isspace(int c)
{
    return c == ' ';
}
