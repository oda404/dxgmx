
#include<dxgmx/attrs.h>
#include<dxgmx/time.h>

_ATTR_ALWAYS_INLINE void 
timespec_add(time_t nsec, Timespec *ts)
{
    ts->nsec += nsec;
    if(ts->nsec >= 1000000000)
    {
        ++ts->sec;
        ts->nsec -= 1000000000;
    }
}
