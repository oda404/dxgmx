/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_MEM_MAP_H
#define _DXGMX_MEM_MAP_H

#include<dxgmx/types.h>

#define MMAP_AVAILABLE        1
#define MMAP_RESERVED         2
#define MMAP_ACPI_RECLAIMABLE 3
#define MMAP_NVS              4
#define MMAP_BADRAM           5

#define MMAP_MAX_ENTRIES_CNT  15

typedef struct S_MemoryMapEntry
{
    u64 base;
    u64 size;
    u32 type;
} MemoryMapEntry;

typedef struct S_MemoryMap
{
    uint32_t entries_cnt;
    MemoryMapEntry entries[MMAP_MAX_ENTRIES_CNT];
} MemoryMap;

/* 
 * Initiate the memory map.
 */
void mmap_init();

/*
 * Adds a new entry to the memory map.
 * @param base Where the area begins.
 * @param size How big is the area.
 * @param type Type of area.
 */
void mmap_add_entry(
    u64 base,
    u64 size,
    u32 type
);

int mmap_update_entry_type(
    u64 base,
    u64 size,
    u32 type
);
/*
    Aligns all the available areas' on 'bytes' boundaries,
    shrinking if needed.
    @param bytes Alignment.
*/
void mmap_align_entries(
    u32 bytes
);

/*
 * Get the full memory map.
 */
const MemoryMap *mmap_get_mmap();

/*
 * Print all entries.
*/
void mmap_dump();

#endif // _DXGMX_MEM_MAP_H
