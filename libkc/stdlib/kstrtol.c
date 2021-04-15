
#include<dxgmx/kstdlib.h>
#include<dxgmx/kctype.h>
#include<dxgmx/kerrno.h>
#include<limits.h>

long int kstrtol(const char *str, char **endptr, int base)
{
    // only base 10 for now
    int neg = 0;
    /* assume the initial str is invalid */
    if(endptr)
        *endptr = (char*)str; /* needed voodo to supress warnings */
    
    while(kisspace(*str))
        ++str;
    if(*str == '-')
    {
        neg = 1;
        ++str;
    }
    /* if a digit isn't found after the potential sign return 0 */
    if(!kisdigit(*str))
    {
        errno = EINVAL;
        return 0;
    }

    long int out = 0;

    for(; *str != '\0'; ++str)
    {
        if(!kisdigit(*str))
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
