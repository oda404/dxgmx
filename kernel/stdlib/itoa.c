/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/stdlib.h>

char *__itoa(int n, char *str, int base)
{
    return __ltoa(n, str, base);
}
