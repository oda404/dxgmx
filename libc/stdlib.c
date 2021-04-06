
#include<stdlib.h>
#include<ctype.h>

void *malloc(size_t size)
{
#ifdef __LIBC_FREESTANDING__ /* kspace */
    size = 2;
#else /* user space */

#endif // __LIBC_FRESTANDING__
    return NULL;
}

void free(void *ptr)
{
#ifdef __LIBC_FREESTANDING__ /* kmode */
    ptr = NULL;
#else /* user space */

#endif // __LIBC_FRESTANDING__
}

long int strtol(const char *str, char **endptr, int base)
{
    long int out = 0;
    int neg = 0;
    if(endptr)
        *endptr = str;
    
    while(isspace(*str))
        ++str;
    if(*str == '-')
    {
        neg = 1;
        ++str;
    }
    if(!isdigit(*str)) // no digits can be parsed
    {
        return 0;
    }

    if(endptr)
        *endptr = NULL;

    for(; *str != '\0'; ++str)
    {
        if(!isdigit(*str))
        {
            if(endptr)
                *endptr = str;
            return neg ? -out : out;
        }

        long int tmp = out * 10 + (*str - '0');
        if(tmp < out) // over or under flow based on sign
        {
            return neg ? -(__LONG_MAX__) : __LONG_MAX__;
        }

        out = tmp;
    }

    return neg ? -out : out;
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
