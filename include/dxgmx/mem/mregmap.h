/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_MAP_H
#define _DXGMX_MEM_MAP_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

#define MEM_REGION_READ 1
#define MEM_REGION_WRITE 2
#define MEM_REGION_EXEC 4
#define MEM_REGION_RW (MEM_REGION_READ | MEM_REGION_WRITE)
#define MEM_REGION_RX (MEM REGION_READ | MEM_REGION_EXEC)
#define MEM_REGION_WX (MEM_REGION_WRITE | MEM_REGION_EXEC)
#define MEM_REGION_RWX (MEM_REGION_READ | MEM_REGION_WRITE | MEM_REGION_EXEC)

typedef struct S_MemoryRegion
{
    u64 start;
    u64 size;
    u8 perms;
} MemoryRegion;

typedef struct S_MemoryRegionMap
{
    MemoryRegion* regions;
    size_t regions_size;
    size_t regions_capacity;
} MemoryRegionMap;

bool mregmap_init(size_t regions, MemoryRegionMap* map);
bool mregmap_add_reg(const MemoryRegion* reg, MemoryRegionMap* map);
bool mregmap_rm_reg(u64 start, u64 size, MemoryRegionMap* map);
bool mregmap_align_regs(u16 alignment, MemoryRegionMap* map);

#define FOR_EACH_MEM_REGION(regionsymbol, map)                                 \
    for (const MemoryRegionMap* _map = map; _map; _map = NULL)                 \
        for (MemoryRegion* regionsymbol = _map->regions;                       \
             regionsymbol <= &_map->regions[_map->regions_size - 1];           \
             ++regionsymbol)

#endif // _DXGMX_MEM_MAP_H
