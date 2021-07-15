
#ifndef _DXGMX_MEM_MAP_H
#define _DXGMX_MEM_MAP_H

#include<stdint.h>

#define MMAP_ENTRY_AVAILABLE        1
#define MMAP_ENTRY_RESERVED         2
#define MMAP_ENTRY_ACPI_RECLAIMABLE 3
#define MMAP_ENTRY_NVS              4
#define MMAP_ENTRY_BADRAM           5
#define MMAP_ENTRY_KRESERVED        6

#define MMAP_MAX_ENTRIES_CNT       15

typedef struct S_MemoryMapEntry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
} MemoryMapEntry;

typedef struct S_MemoryMap
{
    uint32_t entries_cnt;
    MemoryMapEntry entries[MMAP_MAX_ENTRIES_CNT];
} MemoryMap;

/* 
 * Initiate the memory map
 */
void mmap_init();

/*
 * Adds a new memory area. 
 * If the new memory area is overlapping with another it will try 
 * to handle it in such a way that the one of type MMAP_ENTRY_AVAILABLE 
 * is resized/removed. If neither are of type MMAP_ENTRY_AVAILABLE it will abandon ship. 
 * @param base Where the area begins.
 * @param size How big is the area.
 * @param type Type of area.
 */
void mmap_entry_add(
    uint64_t base,
    uint64_t size,
    uint32_t type
);

/*
 * Mark an area as MMAP_ENTRY_KRESERVED, meaning it shouldn't be used 
 * because it contains kernel critical stuff, like the kernel itself. 
 * Only chunks or areas of type MMAP_ENTRY_AVAILABLE can be marked as kreserved.
 * @param base Where the area begins.
 * @param size How big is the area.
 * @return 1 if the given chunk falls inside an invalid area, 
 * 2 if the given chunk doesn't fall inside any area,
 * 0 otherwise.
 */
int mmap_area_mark_kreserved(
    uint64_t base,
    uint64_t size
);

/*
    Aligns all the available areas' bases on given bytes,
    shrinking if needed.
    @param bytes Alignment.
*/
void mmap_entries_align(
    uint32_t bytes
);

/*
 * Get the full memory map.
 */
const MemoryMap *mmap_get_mmap();

/*
 * Print all entries.
*/
void mmap_print();

#endif // _DXGMX_MEM_MAP_H
