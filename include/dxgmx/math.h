/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MATH_H
#define _DXGMX_MATH_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

double modf(double x, double* whole);
float modff(float x, float* whole);
long double modfl(long double x, long double* whole);

_ATTR_PURE
double pow(double x, double y);

_ATTR_PURE
double ceil(double x);

_ATTR_PURE
double floor(double x);

_ATTR_PURE
double exp(double d);

_ATTR_PURE
double log(double x);

_ATTR_PURE
double max(double x, double y);

_ATTR_PURE
double min(double x, double y);

_ATTR_PURE
size_t minzu(size_t x, size_t y);

#endif //!_DXGMX_MATH_H
