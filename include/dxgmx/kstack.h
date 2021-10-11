
#ifndef _DXGMX_KSTACK_H
#define _DXGMX_KSTACK_H

#include<dxgmx/types.h>
#include<dxgmx/attrs.h>

/* The size of the kernel stack in bytes. */
#define KSTACK_SIZE (1024 * 4)

ptr kstack_get_top();
ptr kstack_get_bot();

#endif //!_DXGMX_KSTACK_H
