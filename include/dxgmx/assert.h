/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_ASSERT_H
#define _DXGMX_ASSERT_H

#include<dxgmx/panic.h>

#define ASSERT(x) \
if(!(x))            \
{                 \
    panic("Assertion failed '%s' in %s at %s:%d.", #x, __FILE__, __FUNCTION__, __LINE__); \
}

#endif //_DXGMX_ASSERT_H_
