/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_ABANDON_SHIP_H
#define _DXGMX_ABANDON_SHIP_H

#include<dxgmx/compiler_attrs.h>

/* The Unix equivalent of panic. */
_ATTR_NORETURN _ATTR_FMT_PRINTF(1, 2) void 
abandon_ship(const char *lastwords, ...);

#endif // _DXGMX_ABANDON_SHIP_H
