/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/math.h>

double pow(double x, double y)
{
    const double initx = x;
    while(y--)
        x *= initx;
    return x;
}
