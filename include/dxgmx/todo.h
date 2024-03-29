/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TODO_H
#define _DXGMX_TODO_H

#include <dxgmx/klog.h>
#include <dxgmx/panic.h>

#define TODO()                                                                 \
    klogln(                                                                    \
        WARN,                                                                  \
        "Hit TODO in \"%s\" at %s():%d.",                                      \
        __FILE__,                                                              \
        __FUNCTION__,                                                          \
        __LINE__)

#define TODO_FATAL()                                                           \
    panic(                                                                     \
        "Hit TODO_FATAL in \"%s\" at %s():%d.",                                \
        __FILE__,                                                              \
        __FUNCTION__,                                                          \
        __LINE__)

#endif //_DXGMX_TODO_H
