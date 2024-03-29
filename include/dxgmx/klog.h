/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KLOG_H
#define _DXGMX_KLOG_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>
#include <stdarg.h>

/**
 * When setting a log level, only it and levels who'se values
 * are smaller than it's value will be logged.
 * If the log level is set to 0, nothing will be logged.
 * For example, setting the log level to WARN means that
 * calling klog with KLOG_(WARN/ERR/FATAL) will work but calling
 * it with INFO will not.
 */
typedef enum E_KLogLevel
{
    QUIET = 0,
    FATAL,
    ERR,
    WARN,
    INFO,
    DEBUG,

    KLOG_ENUM_END // do not remove
} KLogLevel;

int klog_init();
/* Early mode is when klog doesn't yet have a timer
and doesn't display info about the time. */
int klog_set_max_level(KLogLevel lvl);

_ATTR_FMT_PRINTF(2, 3) size_t klogln(KLogLevel lvl, const char* fmt, ...);

_ATTR_FMT_PRINTF(2, 3) size_t klog(KLogLevel lvl, const char* fmt, ...);

size_t kvlog(KLogLevel lvl, const char* fmt, va_list list);
size_t kvlogln(KLogLevel lvl, const char* fmt, va_list list);

void klog_set_show_ts(bool show);

/* Convenience macro for putting a prefix before the given fmt.
To use this macro you must define KLOGF_PREFIX to a literal string
before expanding it. */
#define KLOGF(lvl, fmt, ...)                                                   \
    klogln(lvl, KLOGF_PREFIX fmt __VA_OPT__(, ) __VA_ARGS__)

#endif //_DXGMX_KLOG_H
