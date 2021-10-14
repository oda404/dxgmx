
#include<dxgmx/math.h>
#include<dxgmx/klog.h>

double ceil(double x)
{
    return (int)x == x ? x : (int)(x + 1);
}
