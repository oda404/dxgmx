/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_MATH_H
#define _DXGMX_MATH_H

#include<dxgmx/attrs.h>

double modf(double x, double *whole);
double pow(double x, double y)
_ATTR_PURE;

#endif //!_DXGMX_MATH_H
