/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/klog.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/timer.h>
#include<dxgmx/attrs.h>

static Timer g_timer;
static bool g_early = true;
static KLogLevel g_loglvl;

_INIT int klog_init(KLogLevel lvl)
{
    klog_set_max_level(lvl);
    g_early = timer_start(&g_timer) != TIMER_START_OK;

    return 0;
}

bool klog_try_exit_early()
{
    g_early = timer_start(&g_timer) != TIMER_START_OK;
    return !g_early;
}

_INIT int klog_set_max_level(KLogLevel lvl)
{
    if(lvl >= KLOG_ENUM_END)
        lvl = KLOG_ENUM_END - 1;

    g_loglvl = lvl;
    return lvl;
}

size_t kvlog(KLogLevel lvl, const char *fmt, va_list list)
{
    if(lvl > g_loglvl || lvl == 0)
        return 0;

    size_t written = 0;
    if(UNLIKELY(g_early))
        written += kprintf("[   EARLY   ] ");
    else
        written += kprintf("[%11.6f] ", timer_ellapsed_sec(&g_timer));

    return written + kvprintf(fmt, list);
}

size_t kvlogln(KLogLevel lvl, const char *fmt, va_list list)
{
    size_t written = kvlog(lvl, fmt, list);
    written += kprintf("\n");
    return written;
}

size_t klog(KLogLevel lvl, const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlog(lvl, fmt, valist);
    va_end(valist);

    return written;
}

size_t klogln(KLogLevel lvl, const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlogln(lvl, fmt, valist);
    va_end(valist);

    return written;
}
