
#include<dxgmx/time.h>
#include<dxgmx/todo.h>

char *asctime(const struct tm *timeptr)
{
    (void)timeptr;
    TODO_FATAL();
    return NULL;
}

clock_t clock()
{
    TODO_FATAL();
    __builtin_unreachable();
}

char *ctime(const time_t *timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

double difftime(time_t time1, time_t time2)
{
    (void)time1;
    (void)time2;
    TODO_FATAL();
    __builtin_unreachable();
}

struct tm *gmtime(const time_t *timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

struct tm *localtime(const time_t *timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}

time_t mktime(struct tm *timeptr)
{
    (void)timeptr;
    TODO_FATAL();
    __builtin_unreachable();
}

size_t strftime(char *str, size_t maxsize, const char *fmt, const struct tm *timeptr)
{
    (void)str;
    (void)maxsize;
    (void)fmt;
    (void)timeptr;
    TODO_FATAL();
    __builtin_unreachable();
}

time_t time(time_t *timer)
{
    (void)timer;
    TODO_FATAL();
    __builtin_unreachable();
}
