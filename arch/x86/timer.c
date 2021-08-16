
#include<dxgmx/timer.h>
#include<dxgmx/string.h>
#include<dxgmx/x86/rtc.h>

int timer_init(Timer *timer)
{
    memset(timer, 0, sizeof(Timer));
    timer->internaltype = TIMER_INTERNAL_RTC;

    switch(timer->internaltype)
    {
    case TIMER_INTERNAL_RTC:
        timer->ts_getter = rtc_get_running_ts;
        break;
    }

    return timer->internaltype;
}

void timer_start(Timer *timer)
{
    timer->start_ts = *timer->ts_getter();
}

double timer_get_ellapsed_sec(const Timer *timer)
{
    struct timespec now_ts = *timer->ts_getter();
    double now = now_ts.tv_sec + now_ts.tv_nsec / 1000000000.0;
    double start = timer->start_ts.tv_sec + timer->start_ts.tv_nsec / 1000000000.0;
    return now - start;
}

double timer_get_ellapsed_msec(const Timer *timer)
{
    struct timespec now_ts = *timer->ts_getter();
    double now = now_ts.tv_sec * 1000.0 + now_ts.tv_nsec / 1000000.0;
    double start = timer->start_ts.tv_sec * 1000.0 + timer->start_ts.tv_nsec / 1000000.0;
    return now - start;
}

double timer_get_ellapsed_usec(const Timer *timer)
{
    struct timespec now_ts = *timer->ts_getter();
    double now = now_ts.tv_sec * 1000000.0 + now_ts.tv_nsec / 1000.0;
    double start = timer->start_ts.tv_sec * 1000000.0 + timer->start_ts.tv_nsec / 1000.0;
    return now - start;
}

double timer_get_ellapsed_nsec(const Timer *timer)
{
    struct timespec now_ts = *timer->ts_getter();
    double now = now_ts.tv_sec * 1000000000.0 + now_ts.tv_nsec;
    double start = timer->start_ts.tv_sec * 1000000000.0 + timer->start_ts.tv_nsec;
    return now - start;
}
