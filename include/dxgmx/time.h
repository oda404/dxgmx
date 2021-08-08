
#ifndef _DXGMX_TIME_H
#define _DXGMX_TIME_H

#include<stddef.h>
#include<dxgmx/types.h>

#define CLOCKS_PER_SEC 1000000

typedef i64 clock_t;
typedef i64 time_t;

struct tm {
   int tm_sec;
   int tm_min;
   int tm_hour;
   int tm_mday;
   int tm_mon;
   int tm_year;
   int tm_wday;
   int tm_yday;
   int tm_isdst;
};

#endif //_DXGMX_TIME_H
