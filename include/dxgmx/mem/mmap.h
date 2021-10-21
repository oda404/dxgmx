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

#if _PTR_DIG == 8
#   define _PTR_FMT "0x%08lX"
#elif _PTR_DIG == 16
#   define _PTR_FMT "0x%016lX"
#endif

#define _MEM_AREA_FMT "[mem " _PTR_FMT "-" _PTR_FMT "]"

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
    u32 typeu64
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

bool mmap_is_addr_inside_entry(ptr addr, const MemoryMapEntry *entry);

#define FOR_EACH_MMAP_ENTRY(entry) \
for( \
    const MemoryMapEntry *entry = mmap_get_mmap()->entries; \
    (ptr)entry < (ptr)mmap_get_mmap()->entries + mmap_get_mmap()->entries_cnt * sizeof(MemoryMapEntry); \
    ++entry \
)

#endif // _DXGMX_MEM_MAP_H
