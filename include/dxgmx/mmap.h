
#ifndef __DXGMX_MMAP_H__
#define __DXGMX_MMAP_H__

#include<stdint.h>

#define MMAP_AREA_AVAILABLE        1
#define MMAP_AREA_RESERVED         2
#define MMAP_AREA_ACPI_RECLAIMABLE 3
#define MMAP_AREA_NVS              4
#define MMAP_AREA_BADRAM           5
#define MMAP_AREA_KRESERVED        6

typedef struct S_MemoryMapArea
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
} MemoryMapArea;

typedef struct S_MemoryMap
{
    uint32_t areas_cnt;
    MemoryMapArea *areas;
} MemoryMap;

/* 
 * Initiate the memory map
 * @param areas_base Where the info about the memory areas themselves should be stored in memory. It should point to a future area of type MMAP_AREA_AVAILABLE, else mmap_add_area will abandon_ship.
 */
void mmap_init(uint64_t areas_base);

/*
 * Adds a new memory area. Doesn't check if two memory areas are overlapping.
 * @param base Where the area begins.
 * @param size How big is the area.
 * @param type Type of area.
 */
void mmap_add_area(
    uint64_t base,
    uint64_t size,
    uint32_t type
);

/*
 * Mark an area as MMAP_AREA_KRESERVED, meaning it shouldn't be used 
 * because it contains kernel critical stuff, like the kernel itself. 
 * Only chunks or areas of type MMAP_AREA_AVAILABLE can be marked as kreserved.
 * @param base Where the area begins.
 * @param size How big is the area.
 * @return 1 if the given chunk falls inside an invalid area, 
 * 2 if the given chunk doesn't fall inside any area,
 * 0 otherwise.
 */
int mmap_mark_area_kreserved(
    uint64_t base,
    uint64_t size
);
/*
 * Get the full memory map.
 */
const MemoryMap *mmap_get_full_map();

#endif // __DXGMX_MMAP_H__
