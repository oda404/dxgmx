/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TIMEOUT_H
#define _DXGMX_TIMEOUT_H

#include <dxgmx/timer.h>

#define TIMEOUT_CREATE(_name, _duration_ms)                                    \
    static Timer _g_##_name##_timeout;                                         \
    static double _g_##_name##_timeout_dur_ms = _duration_ms;

#define TIMEOUT_START(_name) timer_start(&_g_##_name##_timeout)

#define TIMEOUT_DONE(_name)                                                    \
    (timer_elapsed_ms(&_g_##_name##_timeout) > _g_##_name##_timeout_dur_ms)

#endif // !_DXGMX_TIMEOUT_H
