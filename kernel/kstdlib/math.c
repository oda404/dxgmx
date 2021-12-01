
#include<dxgmx/math.h>
#include<dxgmx/types.h>

double ceil(double x)
{
    return (int)(x + 1);
}

double floor(double x)
{
    return (int)x;
}

double modf(double x, double *whole)
{
    *whole = (doublewhole)x;
    return x - *whole;
}

float modff(float x, float *whole)
{
    *whole = (floatwhole)x;
    return x - *whole;
}

long double modfl(long double x, long double *whole)
{
    *whole = (longdoublewhole)x;
    return x - *whole;
}

double pow(double x, double y)
{
    const double initx = x;
    while(y--)
        x *= initx;
    return x;
}
