/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/ctype.h>

int __isdigit(int c)
{
    // basic ass ascii check
    return c >= '0' && c <= '9';
}
