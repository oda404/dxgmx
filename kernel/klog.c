/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kprintf.h>
#include <dxgmx/timer.h>

static Timer g_timer;
static KLogLevel g_loglvl;

_INIT int klog_init(KLogLevel lvl)
{
    klog_set_max_level(lvl);
    timer_start(&g_timer);

    return 0;
}

_INIT int klog_set_max_level(KLogLevel lvl)
{
    if (lvl >= KLOG_ENUM_END)
        lvl = KLOG_ENUM_END - 1;

    g_loglvl = lvl;
    return lvl;
}

size_t kvlog(KLogLevel lvl, const char* fmt, va_list list)
{
    if (lvl > g_loglvl)
        return 0;

    double time = timer_ellapsed_sec(&g_timer);
    if (UNLIKELY(time < 0))
    {
        time = 0;
        timer_start(&g_timer);
    }

    size_t written = kprintf("[%11.6f] ", time) + kvprintf(fmt, list);

    return written;
}

size_t kvlogln(KLogLevel lvl, const char* fmt, va_list list)
{
    size_t written = kvlog(lvl, fmt, list);
    if (written)
        written += kprintf("\n");
    return written;
}

size_t klog(KLogLevel lvl, const char* fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlog(lvl, fmt, valist);
    va_end(valist);

    return written;
}

size_t klogln(KLogLevel lvl, const char* fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlogln(lvl, fmt, valist);
    va_end(valist);

    return written;
}
