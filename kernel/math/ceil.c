
#include<dxgmx/math.h>

double ceil(double x)
{
    return (int)x == x ? x : (int)(x + 1);
}
