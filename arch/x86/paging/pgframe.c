
#include<dxgmx/pgframe.h>
#include<dxgmx/mmap.h>
#include<dxgmx/stdio.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/bitwise.h>
#include<stdint.h>
#include<stddef.h>
#include<limits.h>

#define PAGE_FRAME_SIZE 4096

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

static uint64_t k_pgframe_pool[1024];
static uint32_t pgframes_avail_cnt = 0;
static const MemoryMap *mmap_sys;

/* Adds any complete PAGE_FRAME_SIZE sized frames from the given area. */
static void pgframe_add_available(const MemoryMapEntry *area)
{
    if(area->type != MMAP_ENTRY_AVAILABLE)
        return;

    for(
        uint64_t frame = area->base;
        frame < area->base + area->size;
        frame += PAGE_FRAME_SIZE
    )
    {
        if(frame > PAGE_FRAME_SIZE * 1024 * 64)
        {
            kprintf("Tried to add out of range page with base 0x%X\n", frame);
            return;
        }
        pgframe_free(frame);
    }
}

void pgframe_alloc_init()
{
    mmap_sys = mmap_get_mmap();

    /* 
     * mark all the pages as being used initially,
     * we will later mark some free as needed.
     */
    for(uint16_t i = 0; i < 1024; ++i)
    {
        k_pgframe_pool[i] = UINT64_MAX;
    }

    for(size_t i = 0; i < mmap_sys->entries_cnt; ++i)
    {
        pgframe_add_available(&mmap_sys->entries[i]);
    }

    if(pgframes_avail_cnt == 0)
        abandon_ship("No free page frames have been registered.\n");

    kprintf(
        "Using %d free, %d byte sized page frames.\n",
        pgframes_avail_cnt,
        PAGE_FRAME_SIZE
    );
}

/* Returns a free page's address */
uint64_t pgframe_alloc()
{
    for(size_t i = 0; i < 1024; ++i)
    {
        uint64_t *pgframe_pool = &k_pgframe_pool[i];
        for(uint16_t k = 0; k < 64; ++k)
        {
            if(!(*pgframe_pool >> k & 1))
            {
                bw_set(pgframe_pool, k);
                --pgframes_avail_cnt;
                return i * 64 * PAGE_FRAME_SIZE + k * PAGE_FRAME_SIZE;
            }
        }
    }
    return 0;
}

uint32_t pgframe_get_avail_frames_cnt()
{
    return pgframes_avail_cnt;
}

uint32_t pgframe_get_frame_size()
{
    return PAGE_FRAME_SIZE;
}

void pgframe_free(uint64_t pgframe_base)
{
    uint64_t pgframe_n = pgframe_base / PAGE_FRAME_SIZE;
    uint16_t pgframe_pool_i = pgframe_n / 64;
    pgframe_n -= pgframe_pool_i * 64;

    bw_clear(
        &k_pgframe_pool[pgframe_pool_i], 
        pgframe_n
    );
    ++pgframes_avail_cnt;
}
