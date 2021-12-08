
#include<dxgmx/math.h>
#include<dxgmx/types.h>

static inline u64 factorial(u64 n)
{
    u64 ret = 1;
    while(n)
    {
        ret *= n;
        --n;
    }
    return ret;
}

static inline double ullpower(double x, size_t y)
{
    double ret = 1;
    while(y--)
        ret *= x;
    return ret;
}

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

double exp(double d)
{
    const size_t epsilon = 20;

    double ret = 0;
    // taylor series.
    for(size_t i = 0; i < epsilon; ++i)
        ret += ullpower(d, i) / factorial(i);

    return ret;
}

double log(double x)
{
    double yn = x - 1.0; // using the first term of the taylor series as initial-value
    double yn1 = yn;
    const double epsilon = 0.01;

    do
    {
        yn = yn1;
        yn1 = yn + 2 * (x - exp(yn)) / (x + exp(yn));
    } while ((yn - yn1) < 0 ? -(yn - yn1) : (yn - yn1) > epsilon);
    return yn1;
}

long double modfl(long double x, long double *whole)
{
    *whole = (longdoublewhole)x;
    return x - *whole;
}

double pow(double x, double y)
{
    /* If the exponent is whole, do it by multiplication. */
    if((u64)y == y)
        return ullpower(x, (u64)y);
    return exp(y * log(x));
}
