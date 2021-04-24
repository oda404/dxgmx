/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/stdlib.h>

char *__itoa(int n, char *str, int base)
{
    if(!n)
    {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    size_t revstart = 0;
    if(n < 0)
    {
        str[0] = '-';
        n = __abs(n);
        ++revstart;
    }

    char tmpbuff[10];
    size_t digits = 0;
    while(n)
    {
        int tmp = n % base;
        if(tmp < 10)
            tmpbuff[digits] = tmp + '0';
        else
            tmpbuff[digits] = tmp + ('A' - 10);
        ++digits;
        n /= base;
    }

    //reverse
    revstart += digits;
    while(digits)
    {
        str[revstart - digits] = tmpbuff[digits - 1];
        --digits;
    }
    str[revstart - digits] = '\0';
    return str;
}