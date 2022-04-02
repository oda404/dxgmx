/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _TIME_H
#define _TIME_H

#include <posix/sys/types.h>

struct timespec
{
    time_t tv_sec;
    long tv_nsec;
};

#endif // !_TIME_H
