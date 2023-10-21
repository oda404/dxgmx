/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/stdlib.h>
#include <dxgmx/string.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/timer.h>
#include <dxgmx/todo.h>

struct timespec timer_stub_now()
{
    return (struct timespec){0};
}

int timer_start(Timer* t)
{
    const TimeSource* ts = timekeep_get_best_timesource();

    t->now = ts->now;
    t->starting_ts = t->now();

    return 0;
}

int timer_elapsed(struct timespec* ts, const Timer* t)
{
    const struct timespec now = t->now();

    ts->tv_sec = now.tv_sec - t->starting_ts.tv_sec;

    time_t nsecdiff = now.tv_nsec - t->starting_ts.tv_nsec;
    if (nsecdiff < 0)
    {
        --ts->tv_sec;
        nsecdiff = 1000000000 + nsecdiff;
    }
    ts->tv_nsec = nsecdiff;

    return 0;
}

double timer_elapsed_sec(const Timer* t)
{
    struct timespec ts;
    timer_elapsed(&ts, t);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

double timer_elapsed_ms(const Timer* t)
{
    struct timespec ts;
    timer_elapsed(&ts, t);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000.0;
}
