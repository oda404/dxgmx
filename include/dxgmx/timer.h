/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_TIMER_H
#define _DXGMX_TIMER_H

#include<dxgmx/time.h>
#include<dxgmx/types.h>

typedef enum
E_TimerInternalType
{
    TIMER_INTERNAL_INVALID = -1,
    TIMER_INTERNAL_RTC,
} TimerInternalType;

typedef const struct timespec*(*timer_ts_getter)();

typedef struct
S_Timer
{
    u8 internaltype;
    struct timespec start_ts;
    timer_ts_getter ts_getter;
} Timer;

/**
 * Initiate the timer.
*/
int timer_init(Timer *timer);
/**
 * Initializes the start_ts member using internal methods.
*/
void timer_start(Timer *timer);
/* Gets the time ellapsed since the last timer_start call. */
double timer_get_ellapsed_sec(const Timer *timer);
double timer_get_ellapsed_msec(const Timer *timer);
double timer_get_ellapsed_usec(const Timer *timer);
double timer_get_ellapsed_nsec(const Timer *timer);

#endif //!_DXGMX_TIMER_H
