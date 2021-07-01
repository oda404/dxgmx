/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_ABANDON_SHIP_H__
#define __DXGMX_ABANDON_SHIP_H__

#include<dxgmx/attrs.h>

/* immediately halts the cpu */
void abandon_ship(const char *lastwords)
_ATTR_NORETURN;

#endif // __DXGMX_ABANDON_SHIP_H__
