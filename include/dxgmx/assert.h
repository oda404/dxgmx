/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_ASSERT_H
#define _DXGMX_ASSERT_H

#include<dxgmx/abandon_ship.h>

#define ASSERT(x) \
if(!(x))            \
{                 \
    abandon_ship("Assertion failed '%s' in %s at %s:%d.\n", #x, __FILE__, __FUNCTION__, __LINE__); \
}

#endif //_DXGMX_ASSERT_H_
