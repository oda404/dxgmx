
#include<dxgmx/paging/frame.h>
#include<dxgmx/paging/size.h>
#include<dxgmx/mmap.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/bitwise.h>
#include<stdint.h>
#include<stddef.h>
#include<limits.h>

/* 
 * This whole page frame allocator is not really following
 * any strict known allocator scheme, is custom built 
 * and is trash, but where's the fun in following a tutorial.
 */

/* 
 * One 64 bit number is called a page frame pool.
 * Each bit in a 64 bit number represent the availability 
 * of 
 * 1024 of those make a kilo page frame pool, and so on.
 */

static uint64_t k_pageframe_pool[1024];
static uint32_t pageframes_avail_cnt = 0;
static const MemoryMap *mmap_sys;

/* Adds any complete PAGE_FRAME_SIZE sized frames from the given area. */
static void pageframe_add_available(const MemoryMapEntry *area)
{
    if(area->type != MMAP_ENTRY_AVAILABLE)
        return;

    for(
        uint64_t frame = area->base;
        frame < area->base + area->size;
        frame += _PAGE_SIZE
    )
    {
        if(frame > _PAGE_SIZE * 1024 * 64)
        {
            kprintf("Tried to add out of range page with base 0x%llX\n", frame);
            return;
        }
        pageframe_free(frame);
    }
}

void pageframe_alloc_init()
{
    mmap_sys = mmap_get_mmap();

    /* 
     * mark all the pages as being used initially,
     * we will later mark some free as needed.
     */
    for(uint16_t i = 0; i < 1024; ++i)
    {
        k_pageframe_pool[i] = UINT64_MAX;
    }

    for(size_t i = 0; i < mmap_sys->entries_cnt; ++i)
    {
        pageframe_add_available(&mmap_sys->entries[i]);
    }

    if(pageframes_avail_cnt == 0)
        abandon_ship("No free page frames have been registered.\n");

    kprintf(
        "Using %ld free, %d byte sized page frames.\n",
        pageframes_avail_cnt,
        _PAGE_SIZE
    );
}

/* Returns a free page's address */
uint64_t pageframe_alloc()
{
    for(size_t i = 0; i < 1024; ++i)
    {
        uint64_t *pageframe_pool = &k_pageframe_pool[i];
        for(uint16_t k = 0; k < 64; ++k)
        {
            if(!(*pageframe_pool >> k & 1))
            {
                bw_set(pageframe_pool, k);
                --pageframes_avail_cnt;
                return i * 64 * _PAGE_SIZE + k * _PAGE_SIZE;
            }
        }
    }
    return 0;
}

uint32_t pageframe_get_avail_frames_cnt()
{
    return pageframes_avail_cnt;
}

void pageframe_free(uint64_t pageframe_base)
{
    uint64_t pageframe_n = pageframe_base / _PAGE_SIZE;
    uint16_t pageframe_pool_i = pageframe_n / 64;
    pageframe_n -= pageframe_pool_i * 64;

    bw_clear(
        &k_pageframe_pool[pageframe_pool_i], 
        pageframe_n
    );
    ++pageframes_avail_cnt;
}
