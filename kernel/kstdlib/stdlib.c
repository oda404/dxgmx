
#include<dxgmx/stdlib.h>
#include<dxgmx/errno.h>
#include<dxgmx/todo.h>
#include<dxgmx/ctype.h>
#include<limits.h>

int abs(int n)
{
    return labs(n);
}

long int labs(long int n)
{
    return llabs(n);
}

long long int llabs(long long int n)
{
    return n < 0 ? -n : n;
}

char *itoa(int n, char *str, int base)
{
    return ltoa(n, str, base);
}

char *ltoa(long n, char *str, int base)
{
    return lltoa(n, str, base);
}

char *lltoa(long long n, char *str, int base)
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
        n = llabs(n);
        ++revstart;
    }

    char tmpbuff[21];
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

char *utoa(unsigned n, char *str, int base)
{
    return ultoa(n, str, base);
}

char *ultoa(unsigned long n, char *str, int base)
{
    return ulltoa(n, str, base);
}

char *ulltoa(unsigned long long n, char *str, int base)
{
    if(!n)
    {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    char tmpbuff[21];
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
    size_t revstart = digits;
    while(digits)
    {
        str[revstart - digits] = tmpbuff[digits - 1];
        --digits;
    }
    str[revstart - digits] = '\0';
    return str;
}
