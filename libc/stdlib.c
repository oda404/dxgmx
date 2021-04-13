
#include<stdlib.h>
#include<ctype.h>
#include<limits.h>
#include<errno.h>

long int strtol(const char *str, char **endptr, int base)
{
    // only base 10 for now
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
    if(!isdigit(*str))
    {
        errno = EINVAL;
        return 0;
    }

    long int out = 0;

    for(; *str != '\0'; ++str)
    {
        if(!isdigit(*str))
            break;

        long int tmp = out * 10 + (*str - '0');
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

char *itoa(int n, char *str, int base)
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
        n = abs(n);
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

void abort(void)
{
    asm volatile ("1: cli; hlt; jmp 1b");
    __builtin_unreachable();
}
