
#include<dxgmx/timer.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/todo.h>
#include<dxgmx/stdlib.h>

u8 g_timersrc = 255;

int timer_find_src()
{
    g_timersrc = TIMERSOURCE_RTC;
    return g_timersrc;
}

void timer_start(Timer *t)
{
    for(size_t i = 0; i < 2; ++i)
    {
        t->src = g_timersrc;
        switch(t->src)
        {
        case TIMERSOURCE_RTC:
            t->now = rtc_now;
            break;

        default:
            timer_find_src();
            continue;
        }
        t->start_ts = t->now();
        return;
    }
    abandon_ship("timer: Failed to find a source. Not proceeding.\n");
}

struct timespec timer_ellapsed(const Timer *t)
{
    if(g_timersrc != t->src)
        TODO_FATAL();

    struct timespec ret;
    const struct timespec now = t->now();

    ret.tv_sec = now.tv_sec - t->start_ts.tv_sec;

    time_t nsecdiff = now.tv_nsec - t->start_ts.tv_nsec;
    if(nsecdiff < 0)
    {
        --ret.tv_sec;
        nsecdiff = 1000000000 + nsecdiff;
    }
    ret.tv_nsec = nsecdiff;

    return ret;
}

double timer_ellapsed_sec(const Timer *t)
{
    const struct timespec ts = timer_ellapsed(t);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

double timer_ellapsed_ms(const Timer *t)
{
    const struct timespec ts = timer_ellapsed(t);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000.0;
}

double timer_ellapsed_us(const Timer *t)
{
    const struct timespec ts = timer_ellapsed(t);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000.0;
}

double timer_ellapsed_ns(const Timer *t)
{
    const struct timespec ts = timer_ellapsed(t);
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
}
