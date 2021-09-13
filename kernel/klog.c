/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/klog.h>
#include<dxgmx/string.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/clocksource.h>

static KLogConfig  g_klogconfig;
static ClockSource g_clocksource;

int klog_init(const KLogConfig *kconfig)
{
    if(!kconfig)
        klog_set_max_level(KLOG_INFO); // default to KLOG_INFO if kconfig is NULL.
    else
        klog_set_max_level(kconfig->loglevel);

    clocksource_init(&g_clocksource);
    clocksource_start(&g_clocksource);

    return 0;
}

int klog_set_max_level(uint8_t lvl)
{
    if(lvl >= KLOG_ENUM_END)
        lvl = KLOG_ENUM_END - 1;
    g_klogconfig.loglevel = lvl;
    return lvl;
}

size_t kvlog(uint8_t lvl, const char *fmt, va_list valist)
{
    if(lvl > g_klogconfig.loglevel || lvl == 0)
        return 0;

    return kprintf(
        "[%f] ", 
        clocksource_ellapsed_sec(&g_clocksource)
    ) + kvprintf(fmt, valist);
}

size_t klog(uint8_t lvl, const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlog(lvl, fmt, valist);
    va_end(valist);

    return written;
}
