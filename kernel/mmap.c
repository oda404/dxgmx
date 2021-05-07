
#include<dxgmx/mmap.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/stdio.h>
#include<stddef.h>

static MemoryMap mmap;
static MemoryMapArea *mareas_area;

void mmap_init(uint64_t areas_base)
{
    mmap.areas_cnt = 0;
    mmap.areas = (MemoryMapArea *)(uint32_t/*suppress warnings*/)areas_base;
    
    /* add an entry marking the entries themselves as kreserved */
    mareas_area = &mmap.areas[0];
    mareas_area->base = areas_base;
    mareas_area->size = sizeof(MemoryMapArea);
    mareas_area->type = MMAP_AREA_KRESERVED;
    ++mmap.areas_cnt;
}

typedef enum E_MemAreaOverlap
{
    OVERLAP_NONE = 0,
    /* First area overlaps at the start of the second area */
    OVERLAP_START = 1,
    /* First area overlaps at the end of the second area */
    OVERLAP_END = 2,
    /* First area is completely inside the second area */
    OVERLAP_COMPLETE = 3
} MemAreaOverlap;

/* 
 * Check if area1 overlaps with area2.
 * @return See MemAreaOverlap enum.
 */
static int mmap_area_overlap(
    const MemoryMapArea *area1,
    const MemoryMapArea *area2
)
{
    if(
        area1->base <= area2->base &&
        area1->base + area1->size < area2->base + area2->size &&
        area1->base + area1->size > area2->base
    )
    {
        return OVERLAP_START;
    }
    else if(
        area1->base > area2->base &&
        area1->base < area2->base + area2->size &&
        area1->base + area1->size >= area2->base + area2->size
    )
    {
        return OVERLAP_END;
    }
    else if(
        area1->base >= area2->base &&
        area1->base + area1->size <= area2->base + area2->size
    )
    {
        return OVERLAP_COMPLETE;
    }
    else
    {
        return OVERLAP_NONE;
    }
}

static int mmap_area_rm(MemoryMapArea *area)
{
    size_t idx = 0;
    size_t i;
    for(i = 1; i < mmap.areas_cnt; ++i)
    {
        MemoryMapArea *tmp = &mmap.areas[i];
        if(
            tmp->base == area->base &&
            tmp->size == area->size && 
            tmp->type == area->type
        )
        {
            idx = i;
            break;
        }
    }
    if(!idx)
        return 1;

    for(i = idx; i < mmap.areas_cnt - 1; ++i)
    {
        mmap.areas[i] = mmap.areas[i + 1];
    }
    --mmap.areas_cnt;
    mmap.areas[0].size -= sizeof(MemoryMapArea);
    return 0;
}

/* 
 * Tries to shrink the mem area at the given index by size.
 * If after shrinking, the area's size is <= 0, it will be removed.
 * @param index Mem area
 * @param size How much to shrink by.
 * @return 1 if area was remove, 0 if it was only shrunk.
 */
static int mmap_area_shrink(
    MemoryMapArea *area, 
    uint64_t size
)
{
    if(size >= area->size)
    {
        mmap_area_rm(area);
        return 1;
    }

    area->size -= size;
    return 0;
}

static int mmap_enlarge(size_t n)
{
    size_t i;
    size_t enlarge_size = n * sizeof(MemoryMapArea);

    /* Check if the enlarged mareas_area will overlap with other areas */
    for(i = 1; i < mmap.areas_cnt; ++i)
    {
        MemoryMapArea *tmp = &mmap.areas[i];
        if(
            mareas_area->base + mareas_area->size + enlarge_size > tmp->base &&
            mareas_area->base + mareas_area->size + enlarge_size <= tmp->base + tmp->size
        )
        {
            if(tmp->type != MMAP_AREA_AVAILABLE)
            {
                kprintf(
                    "Refusing to resize mem area: base 0x%X type %d\n",
                    tmp->base, 
                    tmp->type
                );
                abandon_ship("Illegal memory operation.\n");
            }
            mmap_area_shrink(tmp, enlarge_size);
            tmp->base += enlarge_size;
        }
    }
    mmap.areas_cnt += n; 
    mareas_area->size += enlarge_size;
    return 0;
}

/* 
 * Tries to fix overlaping areas, modifying area1 in such a way that 
 * area2 fits. If needed area1 will be split into multiple areas.
 * Only work if area1 is of type MMAP_AREA_AVAILABLE.
 * @return 0 if successful, 1 if area1 is not of type MMAP_AREA_AVAILABLE, 
 * 2 if area1 and area2 are of the same type and handling is not needed.
 */
static int mmap_areas_handle_overlap(
    MemoryMapArea *area1,
    const MemoryMapArea *area2
)
{
    uint8_t overlap = mmap_area_overlap(area2, area1);
    if(overlap == OVERLAP_NONE)
        return 3;

    if(area1->type == area2->type)
        return 2;
    if(area1->type != MMAP_AREA_AVAILABLE)
        return 1;

    switch(overlap)
    {
    case OVERLAP_START:
        if(mmap_area_shrink(area1, (area2->base + area2->size - area1->base)) == 0)
            area1->base += (area2->base + area2->size - area1->base);
        break;

    case OVERLAP_END:
        mmap_area_shrink(area1, (area2->base + area2->size - area1->base));
        break;

    case OVERLAP_COMPLETE:
    {
        uint64_t init_size = area1->size;
        area1->size = area2->base - area1->base;
        /* first area needs to be split into two */
        mmap_enlarge(1);
        MemoryMapArea *newarea = &mmap.areas[mmap.areas_cnt - 1];
        newarea->type = MMAP_AREA_AVAILABLE;
        newarea->base = area2->base + area2->size;
        newarea->size = init_size - (area2->base + area2->size);
        break;
    }
    }

    return 0;
}

static int mmap_is_addr_inside_area(
    uint64_t addr,
    const MemoryMapArea *area
)
{
    if(addr > area->base && addr < area->base + area->size)
        return 1;
    return 0;
}

static int __is64wide(uint64_t n)
{
    return (n >> 31);
}

void mmap_add_area(
    uint64_t base,
    uint64_t size,
    uint32_t type
)
{
    /* ignore 64 bit values */
    if(__is64wide(base) || __is64wide(size))
        return;
    
    MemoryMapArea *area = &mmap.areas[mmap.areas_cnt];
    area->base = base;
    area->size = size;
    area->type = type;

    ++mmap.areas_cnt;
    mareas_area->size += sizeof(MemoryMapArea);

    for(size_t i = 1; i < mmap.areas_cnt; ++i)
    {
        MemoryMapArea *tmp = &mmap.areas[i];
        if(mmap_areas_handle_overlap(tmp, mareas_area) == 1)
        {
            kprintf(
                "Refusing to fix overlap for (base 0x%X size 0x%X type %d) (base 0x%X size 0x%X type %d)\n",
                area->base,
                area->size,
                area->type,
                tmp->base,
                tmp->size,
                tmp->type
            );
            abandon_ship("Illegal memory operation.\n");
        }
    }

    for(size_t i = 1; i < mmap.areas_cnt - 1; ++i)
    {
        MemoryMapArea *tmp = &mmap.areas[i];
        if(mmap_areas_handle_overlap(tmp, area) == 1)
        {
            kprintf(
                "Refusing to fix overlap for (base 0x%X size 0x%X type %d) (base 0x%X size 0x%X type %d)\n",
                area->base,
                area->size,
                area->type,
                tmp->base,
                tmp->size,
                tmp->type
            );
            abandon_ship("Illegal memory operation.\n");
        }
    }
}

int mmap_mark_area_kreserved(
    uint64_t base,
    uint64_t size
)
{
    MemoryMapArea kreserved;
    kreserved.base = base;
    kreserved.size = size;
    kreserved.type = MMAP_AREA_KRESERVED;

    size_t i;
    for(i = 1; i < mmap.areas_cnt; ++i)
    {
        MemoryMapArea *tmp = &mmap.areas[i];
        mmap_areas_handle_overlap(tmp, &kreserved);
    }

    mmap_enlarge(1);
    mmap.areas[mmap.areas_cnt - 1] = kreserved;

    return 0;
}

void mmap_align_avail_areas(
    uint32_t bytes
)
{
    size_t i;
    for(i = 1; i < mmap.areas_cnt; ++i)
    {
        MemoryMapArea *tmp = &mmap.areas[i];
        if(tmp->type == MMAP_AREA_AVAILABLE)
        {
            uint64_t aligned_base = (tmp->base + (bytes - 1)) & ~(bytes - 1);
            if(mmap_is_addr_inside_area(aligned_base, tmp))
            {
                mmap_area_shrink(tmp, aligned_base - tmp->base);
                tmp->base = aligned_base;
            }
        }
    }
}

const MemoryMap *mmap_get_full_map()
{
    return &mmap;
}
