/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/klog.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/stdio.h>
#include <dxgmx/string.h>
#include <dxgmx/timekeep.h>

static KLogLevel g_loglvl;
static bool g_show_ts = true;

// #define CONFIG_LOGBUF_SIZE 4096
// static char g_logbuffer_buf[CONFIG_LOGBUF_SIZE];
// static RingBuffer g_logbuffer = {
//     .data = g_logbuffer_buf, .max_size = CONFIG_LOGBUF_SIZE, .cursor = 0};

#ifdef CONFIG_KLOG_COLOR
static const char* g_klog_lvl_colors[] = {
    [QUIET] = "",
    [FATAL] = "\x1b[1;31;49m",
    [ERR] = "\x1b[0;31;49m",
    [WARN] = "\x1b[0;33;49m",
    [INFO] = "\x1b[0;39;49m",
    [DEBUG] = "\x1b[0;36;49m",
};
static const char* g_klog_clear_color = "\x1b[0m";
static const char* g_klog_ts_color = "\x1b[0;37;49m";
#endif

static size_t kvprintf(const char* fmt, va_list list)
{
#define KPRINTF_MAX_BUF_SIZE 256

    char buf[KPRINTF_MAX_BUF_SIZE] = {0};
    vsnprintf(buf, KPRINTF_MAX_BUF_SIZE, fmt, list);
    return kstdio_write(buf, strlen(buf));
}

static size_t kprintf(const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);
    size_t written = kvprintf(fmt, list);
    va_end(list);
    return written;
}

_INIT int klog_init()
{
    klog_set_max_level(CONFIG_KLOG_LEVEL);
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

    struct timespec ts = timekeep_uptime();
    double time = ts.tv_sec + ts.tv_nsec / 1000000000.0;

    if (g_show_ts)
    {
#ifdef CONFIG_KLOG_COLOR
        kprintf("%s", g_klog_ts_color);
#endif
        kprintf("%14.5f: ", time);
    }

#ifdef CONFIG_KLOG_COLOR
    kprintf("%s", g_klog_lvl_colors[lvl]);
#endif

    size_t written = kvprintf(fmt, list);

#ifdef CONFIG_KLOG_COLOR
    kprintf("%s", g_klog_clear_color);
#endif
    return written;
}

size_t kvlogln(KLogLevel lvl, const char* fmt, va_list list)
{
    return kvlog(lvl, fmt, list) + kprintf("\n");
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

void klog_set_show_ts(bool show)
{
    g_show_ts = show;
}