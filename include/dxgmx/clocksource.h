
#ifndef _DXGMX_CLOCKSOURCE_H
#define _DXGMX_CLOCKSOURCE_H

#include<dxgmx/types.h>
#include<dxgmx/time.h>

#if defined(_X86_)
#   include<dxgmx/bits/x86/clocksources.h>
#else
#   error "Unknown CPU arch"
#endif // defined(_X86_)

typedef const volatile struct timespec*(*clocksource_ts_getter)();

typedef struct
S_ClockSource
{
    /* Internal architecture specific clock source.
    Examples can be found in include/dxgmx/bits/<arch>/clocksources.h */
    u8 internal_clocksrc;
    /* The starting timespec based on which ellapsed time will
    be calculated. */
    struct timespec start_ts;
    /* Getter for the internal clock source. */
    clocksource_ts_getter internal_ts_getter;
} ClockSource;

int clocksource_init(ClockSource *cs);
void clocksource_start(ClockSource *cs);
double clocksource_ellapsed_sec(const ClockSource *cs);
/* milli */
double clocksource_ellapsed_ms(const ClockSource *cs);
/* micro */
double clocksource_ellapsed_us(const ClockSource *cs);
/* nano */
double clocksource_ellapsed_ns(const ClockSource *cs);

#endif //!_DXGMX_CLOCKSOURCE_H
