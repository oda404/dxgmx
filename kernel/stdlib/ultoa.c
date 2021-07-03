
#include<dxgmx/stdlib.h>

char *__ultoa(unsigned long n, char *str, int base)
{
    return __ulltoa(n, str, base);
}
