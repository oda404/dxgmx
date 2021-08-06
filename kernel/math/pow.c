
#include<dxgmx/math.h>

double pow(double x, double y)
{
    const double initx = x;
    while(y--)
        x *= initx;
    return x;
}
