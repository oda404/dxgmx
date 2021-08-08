/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_ABANDON_SHIP_H
#define _DXGMX_ABANDON_SHIP_H

#include<dxgmx/attrs.h>

/* immediately halts the cpu */
void abandon_ship(const char *lastwords, ...);
_ATTR_FMT_PRINTF(1, 2) _ATTR_NORETURN;

#endif // _DXGMX_ABANDON_SHIP_H
