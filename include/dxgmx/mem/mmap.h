/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_MEM_MAP_H
#define _DXGMX_MEM_MAP_H

#include<dxgmx/types.h>
#include<dxgmx/attrs.h>
#include<dxgmx/mem/memrange.h>

#define MMAP_AVAILABLE        1
#define MMAP_RESERVED         2
#define MMAP_ACPI_RECLAIMABLE 3
#define MMAP_NVS              4
#define MMAP_BADRAM           5

typedef struct S_MemoryMap
{
#define MMAP_MAX_ENTRIES_CNT 15
    size_t entries_cnt;
    MemRangeTyped entries[MMAP_MAX_ENTRIES_CNT];
} MemoryMap;

/* 
 * Initiate the memory map.
 */
_INIT void mmap_init(MemoryMap *mmap);

/*
 * Adds a new entry to the memory map.
 * @param base Where the area begins.
 * @param size How big is the area.
 * @param type Type of area.
 */
_INIT void mmap_add_entry(u64 base, u64 size, u8 type, MemoryMap *mmap);

_INIT int mmap_update_entry_type(u64 base, u64 size, u8 type, MemoryMap *mmap);
/*
    Aligns all the available areas' on 'bytes' boundaries,
    shrinking if needed.
    @param bytes Alignment.
*/
_INIT void mmap_align_entries(u8 type, u32 align, MemoryMap *mmap);

/*
 * Print all entries.
*/
void mmap_dump(const MemoryMap *mmap);

bool mmap_is_addr_inside_entry(u64 addr, const MemRangeTyped *entry);

#define FOR_EACH_MMAP_ENTRY(entry, mmap) \
for( \
    MemRangeTyped *entry = mmap.entries; \
    (ptr)entry < (ptr)mmap.entries + mmap.entries_cnt * sizeof(MemRangeTyped); \
    ++entry \
)

#endif // _DXGMX_MEM_MAP_H
