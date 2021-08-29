/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/klog.h>
#include<dxgmx/string.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/timer.h>

static KLogConfig g_klogconfig;
static Timer      g_klogtimer;

int klog_init(const KLogConfig *kconfig)
{
    if(!kconfig)
        klog_set_max_level(KLOG_INFO); // default to KLOG_INFO if kconfig is NULL.
    else
        klog_set_max_level(kconfig->loglevel);

    timer_init(&g_klogtimer);
    timer_start(&g_klogtimer);

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

    size_t written = 0;

    double sec = timer_get_ellapsed_sec(&g_klogtimer);

    switch(lvl)
    {
    case KLOG_INFO:
        written += kprintf("[%f] ", sec);
        break;

    case KLOG_WARN:
        written += kprintf("[%f] ", sec);
        break;

    case KLOG_ERR:
        written += kprintf("[%f] ", sec);
        break;

    case KLOG_FATAL:
        written += kprintf("[%f] ", sec);
        break;

    default:
        return 0;
    }

    written += kvprintf(fmt, valist);

    return written;
}

size_t klog(uint8_t lvl, const char *fmt, ...)
{
    va_list valist;
    va_start(valist, fmt);
    size_t written = kvlog(lvl, fmt, valist);
    va_end(valist);

    return written;
}
