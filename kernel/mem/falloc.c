/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bitwise.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "falloc: "

/* 1024 * 16 64-bit numbers are enough to hold the
state of 1048576 4KiB pages which hold a total of 4GiB of memory. */
#define PAGEFRAME_POOL_SIZE (1024 * 16)

/* MAYBE FIXME: This whole thing is 128 KiB of memory, which at the time of
 * writting, is about 1/3 of the kernel binary size... */
static u64 g_pgframe_pool[PAGEFRAME_POOL_SIZE];
static size_t g_pgframes_cnt = 0;

/* Adds any complete PAGE_FRAME_SIZE sized frames from the given area. */
static void pageframe_add_available(const MemoryRegion* region)
{
    for (u64 frame = region->start; frame < region->start + region->size;
         frame += PAGESIZE)
        ffree_one(frame);
}

_INIT int falloc_init()
{
    memset(g_pgframe_pool, 0xFF, sizeof(g_pgframe_pool));

    FOR_EACH_MEM_REGION (area, mm_get_sys_mregmap())
        pageframe_add_available(area);

    if (!g_pgframes_cnt)
        panic("falloc: No free page frames have been registered.");

    char unit[4];
    KLOGF(
        INFO,
        "Using %u free %u%s page frames.",
        (size_t)g_pgframes_cnt,
        (size_t)bytes_to_human_readable(PAGESIZE, unit),
        unit);

    return 0;
}

/* Returns a free page's address */
ptr falloc_one()
{
    // TODO: implement some sort of cache so we don't iterate over the whole
    // pool everytime.

    for (ssize_t i = PAGEFRAME_POOL_SIZE - 1; i >= 0; --i)
    {
        u64* pageframe_pool = &g_pgframe_pool[i];

        for (i8 k = 63; k >= 0; --k)
        {
            if (!((*pageframe_pool >> k) & 1))
            {
                bw_set(pageframe_pool, k);
                --g_pgframes_cnt;
                return i * 64 * PAGESIZE + k * PAGESIZE;
            }
        }
    }

    return 0;
}

bool falloc_is_frame_available(ptr base)
{
    ASSERT(base % PAGESIZE == 0);

    u64 bit = base / PAGESIZE;
    u16 i = bit / 64;
    bit -= i * 64;

    bool st = (g_pgframe_pool[i] >> bit) & 1;
    return !st;
}

int falloc_one_at(ptr base)
{
    ASSERT(base % PAGESIZE == 0);

    if (!falloc_is_frame_available(base))
        return -ENOMEM;

    u64 bit = base / PAGESIZE;
    u16 i = bit / 64;
    bit -= i * 64;

    bw_set(&g_pgframe_pool[i], bit);

    return 0;
}

size_t falloc_get_free_frames_count()
{
    return g_pgframes_cnt;
}

void ffree_one(ptr base)
{
    ASSERT(base % PAGESIZE == 0);

    u64 bit = base / PAGESIZE;
    u16 i = bit / 64;
    bit -= i * 64;

    bw_clear(&g_pgframe_pool[i], bit);

    ++g_pgframes_cnt;
}
