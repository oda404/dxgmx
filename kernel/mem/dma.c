/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "dma: "

ERR_OR(ptr) dma_map_range(ptr paddr, size_t n, u16 flags, Process* proc)
{
    ASSERT(paddr % PAGESIZE == 0);

    if (BITMAP_NOT_INIT(proc->dma_bitmap))
    {
        int st = bitmap_init(proc->dma_heap.pagespan, &proc->dma_bitmap);
        if (st < 0)
        {
            KLOGF(ERR, "Failed to initialize bitmap for pid %d", proc->pid);
            return ERR(ptr, st);
        }
    }

    n = bytes_align_up64(n, PAGESIZE);
    const size_t pages = n / PAGESIZE;

    const ssize_t start =
        bitmap_first_n_free_and_mark(pages, &proc->dma_bitmap);
    if (start < 0)
        return ERR(ptr, -ENOMEM);

    for (size_t i = 0; i < pages; ++i)
    {
        int st = mm_map_page(
            proc->dma_heap.vaddr + (start + i) * PAGESIZE,
            paddr + i * PAGESIZE,
            flags,
            proc->paging_struct);

        if (st < 0)
            return ERR(ptr, st);
    }

    return VALUE(ptr, proc->dma_heap.vaddr + start * PAGESIZE);
}
