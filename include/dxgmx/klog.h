/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_KLOG_H
#define _DXGMX_KLOG_H

#include<dxgmx/compiler_attrs.h>
#include<stdint.h>
#include<stddef.h>
#include<stdarg.h>

/** 
 * When setting a log level, only it and levels who'se values
 * are smaller than it's value will be logged.
 * If the log level is set to 0, nothing will be logged.
 * For example, setting the log level to KLOG_WARN means that
 * calling klog with KLOG_(WARN/ERR/FATAL) will work but calling 
 * it with KLOG_INFO will not.
*/
typedef enum E_LogLevels
{
    KLOG_FATAL = 1,
    KLOG_ERR,
    KLOG_WARN,
    KLOG_INFO,

    KLOG_ENUM_END // do not remove
} LogLevels;

typedef struct
S_KLogConfig
{
    uint8_t loglevel;
} KLogConfig;

int klog_init(const KLogConfig *config);
int klog_set_max_level(uint8_t lvl);
size_t klog(uint8_t lvl, const char *fmt, ...)
_ATTR_FMT_PRINTF(2, 3);

size_t kvlog(uint8_t lvl, const char *fmt, va_list list);

#endif //_DXGMX_KLOG_H
