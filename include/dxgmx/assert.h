/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_ASSERT_H
#define _DXGMX_ASSERT_H

#include<dxgmx/panic.h>

#define ASSERT_MSG(x, msg, ...) \
if(!(x)) {                      \
    panic(msg, ##__VA_ARGS__);  \
}

#define ASSERT(x) \
ASSERT_MSG(x, "Assertion failed '%s' in %s at %s:%d.", #x, __FILE__, __FUNCTION__, __LINE__)

#if __STDC_VERSION__ >= 201112L
#   define STATIC_ASSERT(x, msg) _Static_assert(x, msg)
#else
#   warning "C standard is older than C11, STATIC_ASSERT() will get evaluated at run-time"
#   define STATIC_ASSERT(x, msg) ASSERT_MSG(x, msg)
#endif

#endif //_DXGMX_ASSERT_H_
