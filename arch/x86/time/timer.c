
#include<dxgmx/timer.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/todo.h>
#include<dxgmx/string.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/x86/pit.h>

static u8 g_timersrc;
static u8 g_timesrc_ready = false;

static void pit_set_timesource(u8 timersrc)
{
    g_timersrc = timersrc;
    g_timesrc_ready = true;
}

_INIT int timer_find_src()
{
    /* the RTC can only be used as a timer if periodic
    interrupts are enabled. */
    if(pit_periodic_ints_enabled())
    {
        pit_set_timesource(TIMERSOURCE_PIT);
        return 0;
    }
    if(rtc_periodic_ints_enabled())
    {
        pit_set_timesource(TIMERSOURCE_RTC);
        return 0;
    }

    return 1;
}

bool timer_is_ready(const Timer *t)
{
    return t->_is_ready;
}

int timer_start(Timer *t)
{
    memset(t, 0, sizeof(*t));
    t->_is_ready = false;

    if(UNLIKELY(!g_timesrc_ready))
        return TIMER_START_NO_TIMESOURCE;

    t->_src = g_timersrc;
    switch(t->_src)
    {
    case TIMERSOURCE_RTC:
        t->_now = rtc_now;
        break;

    case TIMERSOURCE_PIT:
        t->_now = pit_now;
        break;

    default:
        return TIMER_START_INVALID_TIMESOURCE;
    }

    t->_start_ts = t->_now();
    t->_is_ready = true;

    return TIMER_START_OK;
}

int timer_ellapsed(struct timespec *ts, const Timer *t)
{
    /* If the global timesource is different than the timer's timesource,
    ??? */
    if(UNLIKELY(g_timersrc != t->_src))
        TODO_FATAL();

    if(UNLIKELY( !timer_is_ready(t) ))
        return TIMER_ELLAPSED_NEEDS_STARTING;

    const struct timespec now = t->_now();

    ts->tv_sec = now.tv_sec - t->_start_ts.tv_sec;

    time_t nsecdiff = now.tv_nsec - t->_start_ts.tv_nsec;
    if(nsecdiff < 0)
    {
        --ts->tv_sec;
        nsecdiff = 1000000000 + nsecdiff;
    }
    ts->tv_nsec = nsecdiff;

    return TIMER_ELLAPSED_OK;
}

double timer_ellapsed_sec(const Timer *t)
{
    struct timespec ts; 
    return timer_ellapsed(&ts, t) == TIMER_ELLAPSED_OK ?
        ts.tv_sec + ts.tv_nsec / 1000000000.0 :
        -1;
}

double timer_ellapsed_ms(const Timer *t)
{
    struct timespec ts;
    return timer_ellapsed(&ts, t) == TIMER_ELLAPSED_OK ?
        ts.tv_sec * 1000 + ts.tv_nsec / 1000000.0 :
        -1;
}

double timer_ellapsed_us(const Timer *t)
{
    struct timespec ts;
    return timer_ellapsed(&ts, t) == TIMER_ELLAPSED_OK ?
        ts.tv_sec * 1000000 + ts.tv_nsec / 1000.0 :
        -1;
}

double timer_ellapsed_ns(const Timer *t)
{
    struct timespec ts;
    return timer_ellapsed(&ts, t) == TIMER_ELLAPSED_OK ?
        ts.tv_sec * 1000000000 + ts.tv_nsec :
        -1;
}
