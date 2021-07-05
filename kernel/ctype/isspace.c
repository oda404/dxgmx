/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/ctype.h>

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