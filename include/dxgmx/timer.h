
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
    TimerSources src;
    /* Starting timespec struct. */
    struct timespec start_ts;
    /* Internal getter for timespec structs. */
    timersource now;
} Timer;

int timer_find_src();
void timer_start(Timer *t);
struct timespec timer_ellapsed(const Timer *t);
double timer_ellapsed_sec(const Timer *t);
double timer_ellapsed_ms(const Timer *t);
double timer_ellapsed_us(const Timer *t);
double timer_ellapsed_ns(const Timer *t);


#endif //!_DXGMX_TIMER_H_
