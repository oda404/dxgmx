
#ifndef _DXGMX_KSTACK_H
#define _DXGMX_KSTACK_H

#include<dxgmx/types.h>
#include<dxgmx/attrs.h>

/* The size of the kernel stack in bytes. */
#define _KSTACK_SIZE (1024 * 4)

extern const ptr _kstack_top;
extern const ptr _kstack_bot;

#endif //!_DXGMX_KSTACK_H
