/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_DMA_H
#define _DXGMX_MEM_DMA_H

#include <dxgmx/mem/heap.h>
#include <dxgmx/mem/paging.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/types.h>

/**
 * Maps a physical contiguous range of memory to some virtual contiguous range
 * of memory. The virtual starting address is not up to the caller, but instead
 * chosen internally by the algorithm. You can, however see the memory pool's
 * start and end bounds (proc's dma heap). This memory is of course suitable for
 * doing DMA and/or memory map hardware/
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
