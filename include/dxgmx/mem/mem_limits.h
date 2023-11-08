/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_MEM_LIMITS_H
#define _DXGMX_MEM_MEM_LIMITS_H

#include <dxgmx/generated/kconfig.h>
#include <dxgmx/mem/pagesize.h>

#ifdef CONFIG_32BIT
#define MEM_KERNEL_SPACE_VA_START (3 * GIB)
#define MEM_KERNEL_SPACE_VA_END (4 * GIB)
#else
#error Bad arch
#endif

/* The kernel heap starts after the kernel image virtual end and goes off to the
 * end of the virtual address space. This portion of memory is to be used
 * exclusively through the kmalloc family of functions. */
#define MEM_KERNEL_HEAP_PERC 80
/* A percent of the aforementioned heap can not be used by kmalloc and is
 * reserved for dma. This percent is always taken from the end of the heap. */
#define MEM_KERNEL_DMA_PERC 20

#ifndef _ASM
#include <dxgmx/assert.h>

STATIC_ASSERT(
    MEM_KERNEL_DMA_PERC + MEM_KERNEL_HEAP_PERC == 100,
    "Bad kernel heap and/or DMA dimensions!");

STATIC_ASSERT(
    MEM_KERNEL_SPACE_VA_START % PAGESIZE == 0,
    "Bad alignment for MEM_KERNEL_SPACE_VA_START!");
STATIC_ASSERT(
    MEM_KERNEL_SPACE_VA_END % PAGESIZE == 0,
    "Bad alignment for MEM_KERNEL_SPACE_VA_END!");
#endif

#endif // !_DXGMX_MEM_MEM_LIMITS_H
