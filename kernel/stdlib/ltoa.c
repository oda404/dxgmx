

#include<dxgmx/stdlib.h>

char *__ltoa(long n, char *str, int base)
{
    return __lltoa(n, str, base);
}
