/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_DRIVERS_INPUT_INPUT_USERINPUT_H
#define _DXGMX_DRIVERS_INPUT_INPUT_USERINPUT_H

#include <dxgmx/posix/time.h>
#include <dxgmx/user@types.h>

typedef struct InputEvent
{
    struct timespec time;
    _u8 action;
    _u32 value;
} InputEvent;

#endif // _DXGMX_DRIVERS_INPUT_INPUT_USERINPUT_H
