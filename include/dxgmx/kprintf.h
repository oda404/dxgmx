/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXMGMX_KPRINTF_H
#define _DXMGMX_KPRINTF_H

#include<dxgmx/attrs.h>
#include<stdarg.h>

int kprintf(const char *fmt, ...)
_ATTR_FMT_PRINTF(1, 2) _ATTR_NONNULL(1);

int vkprintf(const char *fmt, va_list arg)
_ATTR_NONNULL(1);

#endif //_DXMGMX_KPRINTF_H
