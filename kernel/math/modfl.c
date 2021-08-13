
#include<dxgmx/math.h>
#include<dxgmx/types.h>

long double modfl(long double x, long double *whole)
{
    *whole = (longdoublewhole)x;
    return x - *whole;
}
