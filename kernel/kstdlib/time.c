/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/time.h>
#include <dxgmx/timer.h>
#include <dxgmx/todo.h>

char* asctime(const struct tm* timeptr)
{
    (void)timeptr;
    TODO_FATAL();
    return NULL;
}

clock_t clock()
{
    TODO_FATAL();
    __builtin_unreachable();
}

char* ctime(const time_t* timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

double difftime(time_t time1, time_t time2)
{
    (void)time1;
    (void)time2;
    TODO_FATAL();
    __builtin_unreachable();
}

struct tm* gmtime(const time_t* timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

struct tm* localtime(const time_t* timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

time_t mktime(struct tm* timeptr)
{
    (void)timeptr;
    TODO_FATAL();
    __builtin_unreachable();
}

size_t
strftime(char* str, size_t maxsize, const char* fmt, const struct tm* timeptr)
{
    (void)str;
    (void)maxsize;
    (void)fmt;
    (void)timeptr;
    TODO_FATAL();
    __builtin_unreachable();
}

time_t time(time_t* timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

int nanosleep(const struct timespec* rqtp, struct timespec* rmtp)
{
#define NANOSLEEP_SECS_OVERHEAD 0.015
    const double secs =
        rqtp->tv_sec + rqtp->tv_nsec / 1000000000.0 - NANOSLEEP_SECS_OVERHEAD;
    Timer t;
    timer_start(&t);

    while (timer_elapsed_sec(&t) < secs)
        ;

    if (rmtp)
    {
        rmtp->tv_nsec = 0;
        rmtp->tv_sec = 0;
    }

    return 0;
}
