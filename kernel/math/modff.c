
#include<dxgmx/math.h>
#include<dxgmx/types.h>

float modff(float x, float *whole)
{
    *whole = (floatwhole)x;
    return x - *whole;
}
