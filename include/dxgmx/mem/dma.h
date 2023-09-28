/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_DMA_H
#define _DXGMX_MEM_DMA_H

#include <dxgmx/mem/paging.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/types.h>

#define DMA_POOL_START (1 * MIB)
#define DMA_POOL_END (PROC_LOW_ADDRESS - 1 * MIB)

int dma_init();

/**
 * Maps a physical contiguous range of memory to some virtual contiguous range
 * of memory. The virtual starting address is not up to the caller, but instead
 * chosen internally by the algorithm. You can, however see the memory pool's
 * start and end bounds (DMA_POOL_START and DMA_POOL_END). This memory is of
 * course suitable for doing DMA, and is only mapped into the kernel's paging
 * struct. It is the mm's job to copy these mappings into user processes.
 *
 * 'paddr' The starting physical address.
 * 'n' How many bytes to map. (Will get page aligned, if not already).
 * 'flags' Page permissions of this mapping. See PAGE_* in mm.h.
 *
 * Returns:
 * ERR_OR(ptr)
 * value: The start of the virtual mapping.
 * error:
 *      -ENOMEM on out of memory.
 */
ERR_OR(ptr) dma_map_range(ptr paddr, size_t n, u16 flags, Process* proc);

#endif // !_DXGMX_MEM_DMA_H
