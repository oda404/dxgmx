
#ifndef _DXGMX_TIMER_H_
#define _DXGMX_TIMER_H_

#include<dxgmx/time.h>

#if defined(_X86_)
#include<dxgmx/bits/x86/timersources.h>
#endif

typedef struct timespec(*timersource)();

typedef struct
S_Timer
{
    /* Internal architecture specific timer source. */
    TimerSources _src;
    /* Starting timespec struct. */
    struct timespec _start_ts;
    /* Internal getter for timespec structs. */
    timersource _now;
    bool _is_ready;
} Timer;

#define TIMER_START_OK 0
#define TIMER_START_NO_TIMESOURCE 1
#define TIMER_START_INVALID_TIMESOURCE 2

#define TIMER_ELLAPSED_OK 0
#define TIMER_ELLAPSED_NEEDS_STARTING 1

/**
 * Tries to find the most suitable source to be used as a timer.
 * @return != 0 if an error occured and no source was found.
*/
int timer_find_src();
/** Starts the timer.
 * @return See TIMER_START_*.
*/
int timer_start(Timer *t);
/**
 * @return true if the timer is valid, and can be used. This status is
 * guaranteed until timer_start() is called again.
*/
bool timer_is_ready(const Timer *t);
/** Puts the ellapsed time since the last timer_start() call 
 in 'ts'. If the returned value != TIMER_ELLAPSED_OK, the value 
 of 'ts' is not touched.
 @return See TIMER_ELLAPSED_*. 
*/
int timer_ellapsed(struct timespec *ts, const Timer *t);
/**
 * @return < 0 if 't' is not ready. Else the number of seconds since
 * timer_start() was last called.
*/
double timer_ellapsed_sec(const Timer *t);
/**
 * @return < 0 if 't' is not ready. Else the number of milliseconds since
 * timer_start() was last called.
*/
double timer_ellapsed_ms(const Timer *t);
/**
 * @return < 0 if 't' is not ready. Else the number of microseconds since
 * timer_start() was last called.
*/
double timer_ellapsed_us(const Timer *t);
/**
 * @return < 0 if 't' is not ready. Else the number of nanoseconds since
 * timer_start() was last called.
*/
double timer_ellapsed_ns(const Timer *t);

#endif //!_DXGMX_TIMER_H_
