
#include<dxgmx/timekeep.h>
#include<dxgmx/x86/pit.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/timer.h>
#include<dxgmx/klog.h>
#include<dxgmx/attrs.h>
#include<dxgmx/todo.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "timekeep: " fmt, ##__VA_ARGS__)

Timer g_uptime_timer;

_INIT int timekeep_init()
{
    /* The PIT should be used as the main system clock until further notice. */
    pit_init();
    pit_enable_periodic_int();

    /* Initialize the RTC for the date/time. */
    rtc_init();
    rtc_enable_periodic_int();

    if(timer_find_src() != 0)
        KLOGF(ERR, "timer found no source even though system timers should be up!");

    timer_start(&g_uptime_timer);

    if(!klog_try_exit_early())
        KLOGF(ERR, "klog could not exit early mode even though system timers should be up!");

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
