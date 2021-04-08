
#include<stdlib.h>
#include<ctype.h>
#include<limits.h>
#include<errno.h>

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
