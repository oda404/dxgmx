/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_PROC_LIMITS_H
#define _DXGMX_PROC_PROC_LIMITS_H

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/types.h>

/* The smallest userspace address. */
#define PROC_LOW_ADDRESS (128UL * MIB)

/* The biggest userspace address.*/
#define PROC_HIGH_ADDRESS (3UL * GIB)

/* Size of a process' kernel stack. */
#define PROC_KSTACK_SIZE (PAGESIZE)

/* Size of a process' stack. */
#define PROC_STACK_SIZE (8 * KIB)

#define PROC_STACK_PAGESPAN (PROC_STACK_SIZE / PAGESIZE)

#endif // !_DXGMX_PROC_PROC_LIMITS_H
