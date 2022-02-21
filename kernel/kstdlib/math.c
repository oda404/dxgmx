
#include<dxgmx/math.h>
#include<dxgmx/stdlib.h>
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
    return (i64)x == x ? (i64)x : (i64)(x + 1);
}

double floor(double x)
{
    return (i64)x;
}

double modf(double x, double *whole)
{
    *whole = (i64)x;
    return x - *whole;
}

long double modfl(long double x, long double *whole)
{
    *whole = (i64)x;
    return x - *whole;
}

float modff(float x, float *whole)
{
    *whole = (i32)x;
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
    } while (abs(yn - yn1) > epsilon);
    return yn1;
}

double pow(double x, double y)
{
    return (i64)y == y ? ullpower(x, (i64)y) : exp(y * log(x));
}

double max(double x, double y)
{
    return x > y ? x : y;
}

double min(double x, double y)
{
    return x > y ? y : x;
}
