
#ifndef _DXGMX_TODO_H
#define _DXGMX_TODO_H

#include<dxgmx/kprintf.h>
#include<dxgmx/abandon_ship.h>

#define TODO() \
kprintf("Hit TODO in %s at %s:%d.", __FILE__, __FUNCTION__, __LINE__);

#define TODO_FATAL() \
abandon_ship("Hit TODO_FATAL in %s at %s:%d.", __FILE__, __FUNCTION__, __LINE__);

#endif //_DXGMX_TODO_H
