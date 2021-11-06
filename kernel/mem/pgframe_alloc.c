/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/pgframe_alloc.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/mem/memrange.h>
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/klog.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/types.h>
#include<dxgmx/utils/bytes.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "pgframe_alloc: " fmt, ##__VA_ARGS__)

/* 
 * This whole page frame allocator is not really following
 * any strict known allocator scheme, is custom built 
 * and is trash, but where's the fun in following a tutorial.
 */

/* 1024 * 16 64-bit numbers are enough to hold the
state of 1048576 4KiB pages which hold a total of 4GiB of memory. */
#define PAGEFRAME_POOL_SIZE (1024 * 16) 
static u64 g_pgframe_pool[PAGEFRAME_POOL_SIZE] = { UINT64_MAX };
static u32 g_pgframes_cnt = 0;

/* Adds any complete PAGE_FRAME_SIZE sized frames from the given area. */
static void pageframe_add_available(const MemRangeTyped *e)
{
    if(e->type != MMAP_AVAILABLE)
        return;

    for(ptr frame = e->base; frame < e->base + e->size; frame += PAGE_SIZE)
    {
        pgframe_free(frame);
    }
}

void pgframe_alloc_init()
{
    FOR_EACH_MMAP_ENTRY(entry, mmanager_get_sys_mmap())
        pageframe_add_available(entry);

    if(g_pgframes_cnt == 0)
        abandon_ship("pgframe_alloc: No free page frames have been registered.\n");

    char unit[4];
    KLOGF(
        INFO, 
        "Using %lu free %d%s page frames.\n",
        g_pgframes_cnt,
        (int)bytes_to_human_readable(PAGE_SIZE, unit),
        unit
    );
}

/* Returns a free page's address */
ptr pgframe_alloc()
{
    // TODO: implement some sort of cache so we don't iterate over the whole pool everytime.
    for(size_t i = 0; i < PAGEFRAME_POOL_SIZE; ++i)
    {
        u64 *pageframe_pool = &g_pgframe_pool[i];
        for(u8 k = 0; k < 64; ++k)
        {
            if(!(*pageframe_pool >> k & 1))
            {
                bw_set(pageframe_pool, k);
                --g_pgframes_cnt;
                return i * 64 * PAGE_SIZE + k * PAGE_SIZE;
            }
        }
    }
    return 0;
}

u32 pgframe_alloc_get_free_pages()
{
    return g_pgframes_cnt;
}

void pgframe_free(ptr frame_base)
{
    u64 pageframe_n = frame_base / PAGE_SIZE;
    u16 pageframe_pool_i = pageframe_n / 64;
    pageframe_n -= pageframe_pool_i * 64;

    bw_clear(
        &g_pgframe_pool[pageframe_pool_i], 
        pageframe_n
    );
    ++g_pgframes_cnt;
}
