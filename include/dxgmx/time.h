/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_TIME_H
#define _DXGMX_TIME_H

/**
 * This file has everything from the libc header 
 * time.h with some added extras like Timespec, Timeval
 * and some helper functions that I ripped off unix spec.
*/

#include<stddef.h>
#include<dxgmx/types.h>

#define CLOCKS_PER_SEC 1000000

typedef i32 clock_t;
typedef i32 time_t;

struct tm {
   int tm_sec;         /* seconds,  range 0 to 59          */
   int tm_min;         /* minutes, range 0 to 59           */
   int tm_hour;        /* hours, range 0 to 23             */
   int tm_mday;        /* day of the month, range 1 to 31  */
   int tm_mon;         /* month, range 0 to 11             */
   int tm_year;        /* The number of years since 1900   */
   int tm_wday;        /* day of the week, range 0 to 6    */
   int tm_yday;        /* day in the year, range 0 to 365  */
   int tm_isdst;       /* daylight saving time             */
};

/* Returns a ptr to a string which represents the date and time of 'timeptr'. */
char *asctime(const struct tm *timeptr);
/* Returns the number of clock ticks since the begining of the program. */
clock_t clock();
/* Returns a ptr to a string which represents the localtime based on 'timer'. */
char *ctime(const time_t *timer);
/* Returns the difference in seconds between 'time1' and 'time2'. */
double difftime(time_t time1, time_t time2);
/* Breaks up timer into the structure tm, expresses it in UTC/GMT and returns it. */
struct tm *gmtime(const time_t *timer);
/* Breaks up timer into the structure tm, expresses it in localtime and returns it. */
struct tm *localtime(const time_t *timer);
/* Converts 'timeptr' into a time_t expressed by the local time zone. */
time_t mktime(struct tm *timeptr);
/* Formats the time represented in 'timeptr' based on the formatting rules of 'fmt' and stores it into 'str'. */
size_t strftime(char *str, size_t maxsize, const char *fmt, const struct tm *timeptr);
/* Encodes the current calendar time into 'timer'. */
time_t time(time_t *timer);

/* Here begin the extras. */

#ifndef __suseconds_t_defined
#ifdef __suseconds_t
typedef __suseconds_t suseconds_t;
#else
typedef i32 __suseconds_t;
typedef __suseconds_t suseconds_t;
#endif __suseconds_t
#define __suseconds_t_defined
#endif // __suseconds_t_defined

typedef struct
S_Timeval
{
    time_t sec;
    suseconds_t usec;
} Timeval;

typedef struct
S_Timespec
{
    time_t sec;
    time_t nsec;
} Timespec;

void timespec_add(time_t nsec, Timespec *ts);

#endif //_DXGMX_TIME_H
