/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/utils/bytes.h>

STATIC_ASSERT(
    DMA_POOL_START % PAGESIZE == 0,
    "DMA pool start must be aligned on page boundary");

STATIC_ASSERT(
    DMA_POOL_END % PAGESIZE == 0,
    "DMA pool end must be aligned on page boundary");

#define KLOGF_PREFIX "dma: "

int dma_init()
{
    size_t dmapool_size = DMA_POOL_END - DMA_POOL_START;
    // size_t dmapool_pages = dmapool_size / PAGESIZE;
    // size_t dmapool_u64s = dmapool_pages / 64;

    char unit[4];
    size_t hr = bytes_to_human_readable(dmapool_size, unit);
    KLOGF(
        INFO,
        "Pool [vmem 0x%p-0x%p] (%u %s)",
        (void*)DMA_POOL_START,
        (void*)DMA_POOL_END,
        hr,
        unit);

    return 0;
}

ERR_OR(ptr) dma_map_range(ptr paddr, size_t n, u16 flags, Process* proc)
{
    if (BITMAP_NOT_INIT(proc->dma_bitmap))
    {
        int st = bitmap_init(128 * MIB / PAGESIZE, &proc->dma_bitmap);
        if (st < 0)
        {
            KLOGF(ERR, "Failed to initialize bitmap for pid %zd", proc->pid);
            return ERR(ptr, st);
        }
    }

    const size_t pages = bytes_align_up64(n, PAGESIZE) / PAGESIZE;
    const ptr aligned_paddr = bytes_align_down64(paddr, PAGESIZE);
    const size_t off = paddr - aligned_paddr;

    ssize_t start = bitmap_first_n_free_and_mark(pages, &proc->dma_bitmap);
    if (start < 0)
        return ERR(ptr, start);

    start = DMA_POOL_START + start * PAGESIZE;

    for (size_t i = 0; i < pages; ++i)
    {
        ptr frame = aligned_paddr + i * PAGESIZE;
        ptr vaddr = start + i * PAGESIZE;

        /* FIXME: maybe we should somehow validate that the physical address
        is free. falloc_one_at does return -ENOMEM if the frame is already
        allocated, but in the case of the multiboot framebuffer, it's base is
        at ~3.9 Gib, which on a system with 128 MiB of RAM, is of cource
        considered "allocated", or a better term would be "unusable" as
        actual RAM. The problem is that falloc does not differentiate between
        the two, and sees such memory as just being allocated. */
        falloc_one_at(frame);

        int st = mm_new_page(vaddr, frame, flags, proc->paging_struct);
        if (st < 0)
            return ERR(ptr, st);
    }

    return VALUE(ptr, start + off);
}
