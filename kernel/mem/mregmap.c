/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/mregmap.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/bitwise.h>
#include <stddef.h>

static bool mregmap_rm_region_at_index(size_t idx, MemoryRegionMap* map)
{
    if (idx >= map->regions_size)
        return false;

    for (size_t i = idx; i < map->regions_size - 1; ++i)
        map->regions[i] = map->regions[i + 1];

    --map->regions_size;
    return true;
}

/** Check to see if the new areas whould go over the map capacity.
 * If they do, try to reallocate the areas array and update the capacity.
 * After the map is enlarged, 'map->regions_size' will be the old count + 'n'.
 * @return true if successful, false if 'areas' couldn't be reallocated.
 */
static bool mregmap_enlarge(size_t n, MemoryRegionMap* map)
{
    if (map->regions_size + n > map->regions_capacity)
    {
        MemoryRegion* tmp = krealloc(map->regions, map->regions_size + n);
        if (!tmp)
            return false;

        map->regions = tmp;
        map->regions_capacity = map->regions_size + n;
    }

    map->regions_size += n;

    return true;
}

#define MEMORY_AREA_SHRINK_OK 0
#define MEMORY_AREA_SHRINK_SHOULD_RM 1

/**
 * Tries to shrink the mem entry by 'size' bytes.
 * @param size How much to shrink by.
 * @param region The region to be shrinked
 * @return 1 if the region is to be removed, 0 if it was shrunk.
 */
static int mreg_shrink(u64 size, MemoryRegion* region)
{
    if (size >= region->size)
        return MEMORY_AREA_SHRINK_SHOULD_RM;

    region->size -= size;
    return MEMORY_AREA_SHRINK_OK;
}

/**
 * @brief Checks to see if 'region' overlaps any other areas from 'map'.
 * If it does the other areas will be modified to allow 'region' to fit.
 * Note that 'region' is not actually inserted into 'map'.
 *
 * @param region The region to be checked for overlaps.
 * @param map The MemoryRegionMap
 */
static void
mregmap_fix_overlaps(const MemoryRegion* region, MemoryRegionMap* map)
{
    for (size_t i = 0; i < map->regions_size; ++i)
    {
        MemoryRegion* tmp = &map->regions[i];
        if (region->start > tmp->start)
        {
            if (region->start + region->size < tmp->start + tmp->size)
            {
                /** The new entry falls inside a secondary entry: 'tmp',
                so 'tmp' is split into two. */
                const u64 prevstart = tmp->start;
                tmp->start = region->start + region->size;
                tmp->size -= tmp->start;

                const MemoryRegion newarea = {
                    .start = prevstart,
                    .size = region->start - prevstart,
                    .perms = tmp->perms};
                mregmap_add_reg(&newarea, map);
            }
            else if (region->start < tmp->start + tmp->size)
            {
                /* The new entry overlaps a second entry: 'tmp'
                at it's end, so we shrink 'tmp'. */
                const size_t shrink = tmp->start + tmp->size - region->start;
                if (mreg_shrink(shrink, tmp) == MEMORY_AREA_SHRINK_SHOULD_RM)
                    mregmap_rm_region_at_index(i, map);
            }
        }
        else
        {
            if (region->start + region->size >= tmp->start + tmp->size)
            {
                /** The new entry completely overlaps a second
                 * entry: 'tmp', so we remove 'tmp'.
                 */
                mregmap_rm_region_at_index(i, map);
            }
            else if (region->start + region->size > tmp->start)
            {
                /** The new entry overlaps another entry: 'tmp' at
                 * it's begining, so we shrink 'tmp' and move it's start.
                 */
                const size_t shrink = region->start + region->size - tmp->start;
                if (mreg_shrink(shrink, tmp) == MEMORY_AREA_SHRINK_OK)
                    tmp->start = region->start + region->size;
                else
                    mregmap_rm_region_at_index(i, map);
            }
        }
    }
}

bool mregmap_init(size_t areas_count, MemoryRegionMap* map)
{
    if (!(map->regions = kmalloc(areas_count * sizeof(MemoryRegion))))
        return false;

    memset(map->regions, 0, areas_count * sizeof(MemoryRegion));
    map->regions_capacity = areas_count;
    map->regions_size = 0;

    return true;
}

bool mregmap_add_reg(const MemoryRegion* region, MemoryRegionMap* map)
{
    /* When a new entry is added it is checked to see if it overlaps
    with any other entries. If overlaps are found the newly added entry
    will NOT be modified, instead the overlapping region(s) will. */
    mregmap_fix_overlaps(region, map);

    if (!mregmap_enlarge(1, map))
        return false;

    map->regions[map->regions_size - 1] = *region;

    /* sort map based on their starts. */
    for (size_t i = 0; i < map->regions_size - 1; ++i)
    {
        for (size_t k = 0; k < map->regions_size - i - 1; ++k)
        {
            if (map->regions[k].start > map->regions[k + 1].start)
            {
                MemoryRegion tmp = map->regions[k];
                map->regions[k] = map->regions[k + 1];
                map->regions[k + 1] = tmp;
            }
        }
    }

    return true;
}

bool mregmap_rm_reg(u64 start, u64 size, MemoryRegionMap* map)
{
    MemoryRegion region = {.start = start, .size = size};
    mregmap_fix_overlaps(&region, map);

    return true;
}

bool mregmap_align_regs(u16 alignment, MemoryRegionMap* map)
{
    if (!bw_is_power_of_two(alignment))
        return false;

    for (size_t i = 0; i < map->regions_size; ++i)
    {
        MemoryRegion* tmp = &map->regions[i];
        u64 aligned_start = (tmp->start + (alignment - 1)) & ~(alignment - 1);

        if (mreg_shrink(aligned_start - tmp->start, tmp) == 1)
        {
            mregmap_rm_region_at_index(i, map);
            break;
        }

        tmp->start = aligned_start;
    }

    return true;
}
