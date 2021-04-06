
#ifndef __DXGMX_ABANDON_SHIP_H__
#define __DXGMX_ABANDON_SHIP_H__

#include<dxgmx/gcc/attrs.h>

/* immediately halts the cpu */
void abandon_ship(const char *lastwords)
__ATTR_NORETURN;

#endif // __DXGMX_ABANDON_SHIP_H__
