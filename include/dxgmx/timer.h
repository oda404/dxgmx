
#ifndef _DXGMX_TIMER_H_
#define _DXGMX_TIMER_H_

#include<dxgmx/time.h>

typedef struct timespec(*timersource_now)();

typedef struct
S_Timer
{
    /* Starting timespec struct. */
    struct timespec _start_ts;
    /* Internal getter for timespec structs. */
    timersource_now _now;
    bool _ready;
    size_t _internal_id;
} Timer;

/**
 * Tries to find the most suitable source to be used as a timer.
 * @return != 0 if an error occured and no source was found.
*/
size_t timer_find_sources();
/** Starts the timer.
 * @return See TIMER_START_*.
*/
bool timer_start(Timer *t);
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
bool timer_ellapsed(struct timespec *ts, const Timer *t);
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
