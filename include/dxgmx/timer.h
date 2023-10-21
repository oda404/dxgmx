/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TIMER_H
#define _DXGMX_TIMER_H

#include <dxgmx/time.h>

#define TIMER_DORMANT                                                          \
    (Timer)                                                                    \
    {                                                                          \
        .now = timer_stub_now, .starting_ts = (struct timespec)                \
        {                                                                      \
            0                                                                  \
        }                                                                      \
    }

typedef struct S_Timer
{
    /* Starting timespec struct. */
    struct timespec starting_ts;
    /* Internal getter for timespec structs. */
    struct timespec (*now)();
} Timer;

/**
 * Start a timer. Once a timer has been started you can get how much time has
 * elapsed since starting it, using the timer_elapsed* functions.
 *
 * 't' The timer.
 *
 * Returns:
 * 0 on success.
 */
int timer_start(Timer* t);

/**
 * Get elapsed time since a timer has been started.
 *
 * No NULL pointers should be passed to this function. Additionally the timer
 * must be started before being passed to this function.
 *
 * 'ts' The destination struct timespec.
 * 't' The Timer.
 *
 * Returns:
 * 0 on success.
 */
int timer_elapsed(struct timespec* ts, const Timer* t);

/**
 * Get elapsed time since a timer has been started in seconds.
 *
 * No NULL pointers should be passed to this function. Additionally the timer
 * must be started before being passed to this function.
 *
 * 'ts' The destination struct timespec.
 * 't' The Timer.
 *
 * Returns:
 * The number of seconds since the timer has been started as a floating point
 * type.
 */
double timer_elapsed_sec(const Timer* t);

/**
 * Get elapsed time since a timer has been started in milliseconds.
 *
 * No NULL pointers should be passed to this function. Additionally the timer
 * must be started before being passed to this function.
 *
 * 'ts' The destination struct timespec.
 * 't' The Timer.
 *
 * Returns:
 * The number of milliseconds since the timer has been started as a floating
 * point type.
 */
double timer_elapsed_ms(const Timer* t);

struct timespec timer_stub_now();

#endif //!_DXGMX_TIMER_H
