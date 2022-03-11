/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KSTACK_H
#define _DXGMX_KSTACK_H

#include <dxgmx/types.h>

/* The size of the kernel stack in bytes. */
#define KSTACK_SIZE (2 * KIB)

extern ptr _kernel_stack_top;
extern ptr _kernel_stack_bot;

#endif //!_DXGMX_KSTACK_H
