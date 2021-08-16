/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_KLOG_H
#define _DXGMX_KLOG_H

#include<dxgmx/attrs.h>
#include<stdint.h>
#include<stddef.h>
#include<stdarg.h>

/** 
 * When you set a log level. Only it and any levels who'se values are
 * bigger than it will get logged.
*/
typedef enum E_LogLevels
{
    KLOG_QUIET,
    KLOG_INFO,
    KLOG_WARN,
    KLOG_ERR,
    KLOG_FATAL,
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
