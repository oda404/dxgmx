/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/klog.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/timer.h>
#include<dxgmx/attrs.h>

static KLogConfig  g_klogconfig;
static Timer g_timer;

_INIT int klog_init(const KLogConfig *kconfig)
{
    if(!kconfig)
        klog_set_max_level(INFO); // default to INFO if kconfig is NULL.
    else
        klog_set_max_level(kconfig->loglevel);

    timer_start(&g_timer);

    return 0;
}

_INIT int klog_set_max_level(KLogLevel lvl)
{
    if(lvl >= KLOG_ENUM_END)
        lvl = KLOG_ENUM_END - 1;
    g_klogconfig.loglevel = lvl;
    return lvl;
}

size_t kvlog(KLogLevel lvl, const char *fmt, va_list list)
{
    if(lvl > g_klogconfig.loglevel || lvl == 0)
        return 0;

    double secs = timer_ellapsed_sec(&g_timer);
    if(UNLIKELY(secs < 0))
    {
        secs = 0;
        /* Try to start the timer once more, for the next kvlog call. */
        timer_start(&g_timer);   
    }

    return kprintf("[%f] ", secs) + kvprintf(fmt, list);
}

size_t klog(KLogLevel lvl, const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlog(lvl, fmt, valist);
    va_end(valist);

    return written;
}
