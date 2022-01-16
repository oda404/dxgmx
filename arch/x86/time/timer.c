
#include<dxgmx/timer.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/todo.h>
#include<dxgmx/string.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/x86/pit.h>
#include<dxgmx/attrs.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "timer: " fmt, ##__VA_ARGS__)

typedef struct 
S_TimerSource
{
    const char* const name;
    const size_t id;
    const u8 priority;
    bool useable;
    const timersource_now now;
} TimerSource;

#define RTC_ID 0
#define PIT_ID 1

static TimerSource g_timersources[] = {
    { .name = "RTC", .id = RTC_ID, .priority = 10, .useable = false, .now = rtc_now },
    { .name = "PIT", .id = PIT_ID, .priority = 20, .useable = false, .now = pit_now },
};

_INIT static TimerSource *find_best_timersource()
{
    TimerSource *best = NULL;

    for(size_t i = 0; i < sizeof(g_timersources) / sizeof(TimerSource); ++i)
    {
        TimerSource *src = &g_timersources[i];
        if(best)
        {
            if(src->priority > best->priority && src->useable)
                best = src;
        }
        else
        {
            if(src->useable)
                best = src;
        }
    }

    return best;
}

_INIT static TimerSource *find_timersource_by_id(size_t id)
{
    for(size_t i = 0; i < sizeof(g_timersources) / sizeof(TimerSource); ++i)
    {
        if(g_timersources[i].id == id)
            return &g_timersources[i];
    }

    return NULL;
}

_INIT size_t timer_find_sources()
{
    size_t useable = 0;

    if(rtc_periodic_ints_enabled())
    {
        TimerSource *timersource = find_timersource_by_id(RTC_ID);
        if(timersource)
        {
            timersource->useable = true;
            ++useable;
        }
    }

    if(pit_periodic_ints_enabled())
    {
        TimerSource *timersource = find_timersource_by_id(PIT_ID);
        if(timersource)
        {
            timersource->useable = true;
            ++useable;
        }
    }

    const TimerSource *best = find_best_timersource();
    if(best)
        KLOGF(INFO, "Found best timersource \"%s\" with id %d.", best->name, best->id);

    return useable;
}

bool timer_is_ready(const Timer *t)
{
    return t->_ready;
}

bool timer_start(Timer *t)
{
    memset(t, 0, sizeof(Timer));

    const TimerSource *best = find_best_timersource();
    if(!best)
        return false;

    t->_now = best->now;
    t->_start_ts = t->_now();
    t->_internal_id = best->id;
    t->_ready = true;

    return true;
}

bool timer_ellapsed(struct timespec *ts, const Timer *t)
{
    if(UNLIKELY( !timer_is_ready(t) ))
        return false;

    const struct timespec now = t->_now();

    ts->tv_sec = now.tv_sec - t->_start_ts.tv_sec;

    time_t nsecdiff = now.tv_nsec - t->_start_ts.tv_nsec;
    if(nsecdiff < 0)
    {
        --ts->tv_sec;
        nsecdiff = 1000000000 + nsecdiff;
    }
    ts->tv_nsec = nsecdiff;

    return true;
}

double timer_ellapsed_sec(const Timer *t)
{
    struct timespec ts; 
    return timer_ellapsed(&ts, t) ?
        ts.tv_sec + ts.tv_nsec / 1000000000.0 :
        -1;
}

double timer_ellapsed_ms(const Timer *t)
{
    struct timespec ts;
    return timer_ellapsed(&ts, t) ?
        ts.tv_sec * 1000 + ts.tv_nsec / 1000000.0 :
        -1;
}

double timer_ellapsed_us(const Timer *t)
{
    struct timespec ts;
    return timer_ellapsed(&ts, t) ?
        ts.tv_sec * 1000000 + ts.tv_nsec / 1000.0 :
        -1;
}

double timer_ellapsed_ns(const Timer *t)
{
    struct timespec ts;
    return timer_ellapsed(&ts, t) ?
        ts.tv_sec * 1000000000 + ts.tv_nsec :
        -1;
}
