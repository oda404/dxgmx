
#include<dxgmx/pgframe.h>
#include<dxgmx/mmap.h>
#include<dxgmx/stdio.h>
#include<dxgmx/abandon_ship.h>
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

void __nonnative_clear_bit(uint64_t *n, uint8_t bit)
{
    if(bit < 32)
    {
        *((uint32_t *)n) &= ~(1 << bit);
    }
    else if(bit < 64)
    {
        uint32_t half = (uint32_t)(*n);
        *n >>= 32;
        *((uint32_t *)n) &= ~(1 << (bit - 32));
        *n <<= 32;
        *((uint32_t *)n) = half;
    }
}

void __nonnative_set_bit(uint64_t *n, uint8_t bit)
{
    if(bit < 32)
    {
        *((uint32_t *)n) |= 1 << bit;
    }
    else if(bit < 64)
    {
        uint32_t half = (uint32_t)(*n);
        *n >>= 32;
        *((uint32_t *)n) |= 1 << (bit - 32);
        *n <<= 32;
        *((uint32_t *)n) = half;
    }
}

/* Adds any complete PAGE_FRAME_SIZE sized frames from the given area. */
static void pgframe_add_available(const MemoryMapArea *area)
{
    if(area->type != MMAP_AREA_AVAILABLE)
        return;

    for(
        uint64_t frame = area->base;
        frame < area->base + area->size;
        frame += PAGE_FRAME_SIZE
    )
    {
        pgframe_free(frame);
    }
}

void pgframe_alloc_init()
{
    mmap_sys = mmap_get_full_map();

    /* 
     * mark all the pages as being used initially,
     * we will later mark some free as needed.
     */
    for(uint16_t i = 0; i < 1024; ++i)
    {
        k_pgframe_pool[i] = UINT64_MAX;
    }

    /* 
     * i lose a bit of available physical memory by aligning 
     * the available areas but gain a lot of mental health
     */
    mmap_align_avail_areas(PAGE_FRAME_SIZE);

    for(size_t i = 0; i < mmap_sys->areas_cnt; ++i)
    {
        pgframe_add_available(&mmap_sys->areas[i]);
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
                __nonnative_set_bit(pgframe_pool, k);
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

    __nonnative_clear_bit(
        &k_pgframe_pool[pgframe_pool_i], 
        pgframe_n
    );
    ++pgframes_avail_cnt;
}
