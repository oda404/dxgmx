/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/timer.h>
#include <dxgmx/todo.h>
#include <dxgmx/x86/pit.h>
#include <dxgmx/x86/rtc.h>

#define KLOGF(lvl, fmt, ...)                                                   \
    klogln(lvl, "timekeep: " fmt __VA_OPT__(, ) __VA_ARGS__)

Timer g_uptime_timer;

_INIT int timekeep_init()
{
    /* The PIT should be used as the main system clock until further notice. */
    pit_init();
    pit_enable_periodic_int();

    /* Initialize the RTC for the date/time. */
    rtc_init();
    rtc_enable_periodic_int();

    if (!timer_find_sources())
        KLOGF(
            ERR, "No system timers were found even though they should be up!");

    timer_start(&g_uptime_timer);

    return 0;
}

struct tm timkeep_date()
{
    return rtc_date();
}

struct timespec timekeep_uptime()
{
    struct timespec ret;
    timer_ellapsed(&ret, &g_uptime_timer);
    return ret;
}

struct timespec timekeep_unixts()
{
    TODO_FATAL();
}
