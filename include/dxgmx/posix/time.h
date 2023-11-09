/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_POSIX_TIME_H
#define _DXGMX_POSIX_TIME_H

#include <dxgmx/posix/sys/types.h>

struct timespec
{
    time_t tv_sec;
    int32_t tv_nsec;
};

#endif // !_DXGMX_POSIX_TIME_H
