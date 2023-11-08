/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_PROC_LIMITS_H
#define _DXGMX_PROC_PROC_LIMITS_H

#include <dxgmx/mem/mem_limits.h>
#include <dxgmx/mem/pagesize.h>

/* The smallest userspace address. */
#define PROC_LOW_ADDRESS (1 * MIB)

/* The biggest userspace address.*/
#define PROC_HIGH_ADDRESS MEM_KERNEL_SPACE_VA_START

/* Size of a process' kernel stack. */
#define PROC_KSTACK_SIZE (1 * PAGESIZE)

/* Size of a process' stack. */
#define PROC_STACK_SIZE (2 * PAGESIZE)

#endif // !_DXGMX_PROC_PROC_LIMITS_H
