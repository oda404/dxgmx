/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_KSTDIO_H__
#define __DXGMX_KSTDIO_H__

#include<dxgmx/attrs.h>

int kprintf(const char *fmt, ...)
_ATTR_FMT_PRINTF(1, 2) _ATTR_NONNULL(1);

#endif // __DXGMX_KSTDIO_H__
