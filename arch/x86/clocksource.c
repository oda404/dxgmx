
#include<dxgmx/clocksource.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/string.h>

static i8 g_clocksrc = -1;

static i8 clocksource_find_best_source()
{
    return CLOCKSOURCE_RTC; // damn
}

int clocksource_init(ClockSource *cs)
{
    memset(cs, 0, sizeof(cs));
    if(g_clocksrc == -1)
        g_clocksrc = clocksource_find_best_source();

    cs->internal_clocksrc = g_clocksrc;
    switch(cs->internal_clocksrc)
    {
    case CLOCKSOURCE_RTC:
        cs->internal_ts_getter = rtc_get_running_ts;
        break;
    }

    return cs->internal_clocksrc;
}

void clocksource_start(ClockSource *cs)
{
    cs->start_ts = *cs->internal_ts_getter();
}

double clocksource_ellapsed_sec(const ClockSource *cs)
{
    struct timespec now_ts = *cs->internal_ts_getter();
    double now = now_ts.tv_sec + now_ts.tv_nsec / 1000000000.0;
    double start = cs->start_ts.tv_sec + cs->start_ts.tv_nsec / 1000000000.0;
    return now - start;
}

/* milli */
double clocksource_ellapsed_ms(const ClockSource *cs)
{
    struct timespec now_ts = *cs->internal_ts_getter();
    double now = now_ts.tv_sec * 1000.0 + now_ts.tv_nsec / 1000000.0;
    double start = cs->start_ts.tv_sec * 1000.0 + cs->start_ts.tv_nsec / 1000000.0;
    return now - start;
}

/* micro */
double clocksource_ellapsed_us(const ClockSource *cs)
{
    struct timespec now_ts = *cs->internal_ts_getter();
    double now = now_ts.tv_sec * 1000000.0 + now_ts.tv_nsec / 1000.0;
    double start = cs->start_ts.tv_sec * 1000000.0 + cs->start_ts.tv_nsec / 1000.0;
    return now - start;
}

/* nano */
double clocksource_ellapsed_ns(const ClockSource *cs)
{
    struct timespec now_ts = *cs->internal_ts_getter();
    double now = now_ts.tv_sec * 1000000000.0 + now_ts.tv_nsec;
    double start = cs->start_ts.tv_sec * 1000000000.0 + cs->start_ts.tv_nsec;
    return now - start;
}
