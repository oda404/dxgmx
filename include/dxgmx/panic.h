/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PANIC_H
#define _DXGMX_PANIC_H

#include <dxgmx/compiler_attrs.h>

/* Architecture specific function to prepare for a kernel panic. */
int panic_arch_prepare();

/* The Unix equivalent of panic. */
_ATTR_NORETURN _ATTR_FMT_PRINTF(1, 2) void panic(const char* lastwords, ...);

#endif // _DXGMX_PANIC_H
