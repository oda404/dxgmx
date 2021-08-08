/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/math.h>
#include<dxgmx/types.h>

double modf(double x, double *whole)
{
    *whole = (i64)x;
    return x - *whole;
}
