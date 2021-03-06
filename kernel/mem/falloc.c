/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mmanager.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bitwise.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "falloc: "

/*
 * This whole page frame allocator is not really following
 * any strict known allocator scheme, is custom built
 * and is trash, but where's the fun in following a tutorial.
 */

/* 1024 * 16 64-bit numbers are enough to hold the
state of 1048576 4KiB pages which hold a total of 4GiB of memory. */
#define PAGEFRAME_POOL_SIZE (1024 * 16)
static u64 g_pgframe_pool[PAGEFRAME_POOL_SIZE];
static u32 g_pgframes_cnt = 0;

/* Adds any complete PAGE_FRAME_SIZE sized frames from the given area. */
static void pageframe_add_available(const MemoryRegion* region)
{
    for (u64 frame = region->start; frame < region->start + region->size;
         frame += PAGE_SIZE)
        ffree(frame, 1);
}

_INIT int falloc_init()
{
    memset(g_pgframe_pool, 0xFF, sizeof(g_pgframe_pool));

    FOR_EACH_MEM_REGION (area, mmanager_get_sys_mregmap())
        pageframe_add_available(area);

    if (!g_pgframes_cnt)
        panic("falloc: No free page frames have been registered.");

    char unit[4];
    KLOGF(
        INFO,
        "Using %lu free %d%s page frames.",
        (unsigned long)g_pgframes_cnt,
        (int)bytes_to_human_readable(PAGE_SIZE, unit),
        unit);

    return 0;
}

/* Returns a free page's address */
ptr falloc(size_t n)
{
    if (!n)
        return 0;

    if (n > 1)
        TODO_FATAL();

    // TODO: implement some sort of cache so we don't iterate over the whole
    // pool everytime.
    for (size_t i = 0; i < PAGEFRAME_POOL_SIZE; ++i)
    {
        u64* pageframe_pool = &g_pgframe_pool[i];
        for (u8 k = 0; k < 64; ++k)
        {
            if (!((*pageframe_pool >> k) & 1))
            {
                bw_set(pageframe_pool, k);
                --g_pgframes_cnt;
                return i * 64 * PAGE_SIZE + k * PAGE_SIZE;
            }
        }
    }
    return 0;
}

size_t falloc_get_free_frames_count()
{
    return g_pgframes_cnt;
}

void ffree(ptr base, size_t n)
{
    if (!n)
        return;

    if (n > 1)
        TODO_FATAL();

    u64 bit = base / PAGE_SIZE;
    u16 i = bit / 64;
    bit -= i * 64;

    bw_clear(&g_pgframe_pool[i], bit);

    ++g_pgframes_cnt;
}
