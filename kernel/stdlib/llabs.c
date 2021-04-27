/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/stdlib.h>

long long int __llabs(long long int n)
{
    return n < 0 ? -n : n;
}
