/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/stdlib.h>
#include<dxgmx/ctype.h>
#include<dxgmx/errno.h>
#include<limits.h>
#include<dxgmx/todo.h>

static int isbasedigit(int c, int base)
{    
    switch(base)
    {
    case 2 ... 10:
        return (c >= '0' && c <= '0' + base - 1);
    case 11 ... 32:
        return 
            (c >= '0' && c <= '9') || 
            (c >= 'A' && c <= 'A' + base - 11) ||
            (c >= 'a' && c <= 'a' + base - 11);
    }

    return 0;
}

long int strtol(const char *str, char **endptr, int base)
{
    if((base < 2 && base != 0) || base > 36)
        return 0;

    if(base == 0)
        TODO_FATAL(); // can't be fucked now
    
    int neg = 0;
    /* assume the initial str is invalid */
    if(endptr)
        *endptr = (char*)str; /* needed voodo to supress warnings */
    
    while(isspace(*str))
        ++str;
    if(*str == '-')
    {
        neg = 1;
        ++str;
    }
    /* if a digit isn't found after the potential sign return 0 */
    if(!isbasedigit(*str, base))
    {
        errno = EINVAL;
        return 0;
    }

    long int out = 0;

    for(; *str != '\0'; ++str)
    {
        if(!isbasedigit(*str, base))
            break;

        long int tmp;
        if(*str <= '9')
            tmp = out * base + (*str - '0');
        else if (*str >= 'a' && *str <= 'v')
            tmp = out * base + (*str - 'a' + 10);
        else
            tmp = out * base + (*str - 'A' + 10);

        if(tmp < out) // over or under flow based on sign
        {
            errno = ERANGE;
            return neg ? LONG_MIN : LONG_MAX;
        }
        out = tmp;
    }

    if(endptr)
        *endptr = (char*)str; /* needed voodo to supress warnings */
    return neg ? -out : out;
}
