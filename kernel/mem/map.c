
#include<dxgmx/mem/map.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/attrs.h>
#include<stddef.h>

static MemoryMap mmap;

void mmap_init()
{
    mmap.entries_cnt = 0;
}

typedef enum E_MemEntryOverlap
{
    MMAP_ENTRY_OVERLAP_NONE = 0,
    /* First area overlaps at the start of the second area */
    MMAP_ENTRY_OVERLAP_START = 1,
    /* First area overlaps at the end of the second area */
    MMAP_ENTRY_OVERLAP_END = 2,
    /* First area is completely inside the second area */
    MMAP_ENTRY_OVERLAP_COMPLETE = 3
} MemEntryOverlap;

/* 
 * Check if entry1 overlaps with entry2. 
 * Note: If entry2 is completely inside entry1 this 
 * function will return MMAP_ENTRY_OVERLAP_NONE 
 * because it only checks if entry1 is completely in entry2.
 * @return See MemEntryOverlap enum.
 */
static int mmap_entry_overlap(
    const MemoryMapEntry *entry1,
    const MemoryMapEntry *entry2
)
{
    if(
        entry1->base <= entry2->base &&
        entry1->base + entry1->size < entry2->base + entry2->size &&
        entry1->base + entry1->size > entry2->base
    )
    {
        return MMAP_ENTRY_OVERLAP_START;
    }
    else if(
        entry1->base >= entry2->base &&
        entry1->base < entry2->base + entry2->size &&
        entry1->base + entry1->size > entry2->base + entry2->size
    )
    {
        return MMAP_ENTRY_OVERLAP_END;
    }
    else if(
        entry1->base >= entry2->base &&
        entry1->base + entry1->size <= entry2->base + entry2->size
    )
    {
        return MMAP_ENTRY_OVERLAP_COMPLETE;
    }
    else
    {
        return MMAP_ENTRY_OVERLAP_NONE;
    }
}

static int mmap_entry_rm(MemoryMapEntry *area)
{
    for(size_t i = 0; i < mmap.entries_cnt; ++i)
    {
        if(&mmap.entries[i] == area)
        {
            for(; i < mmap.entries_cnt - 1; ++i)
            {
                mmap.entries[i] = mmap.entries[i + 1];
            }
            --mmap.entries_cnt;
            mmap.entries[0].size -= sizeof(MemoryMapEntry);
            return 0;
        }
    }
    return 1;
}

/* 
 * Tries to shrink the mem area at the given index by size.
 * If after shrinking, the area's size is <= 0, it will be removed.
 * @param area Mem area
 * @param size How much to shrink by.
 * @return 1 if area was remove, 0 if it was only shrunk.
 */
static int mmap_entry_shrink(
    MemoryMapEntry *entry, 
    uint64_t size
)
{
    if(size >= entry->size)
    {
        mmap_entry_rm(entry);
        return 1;
    }

    entry->size -= size;
    return 0;
}

static int mmap_enlarge(size_t n)
{
    if(mmap.entries_cnt + n > MMAP_MAX_ENTRIES_CNT)
        kprintf("Exceeded MMAP_MAX_ENTRIES_CNT when enlarging mmap.");
    mmap.entries_cnt += n;
    return n;
}

typedef enum E_MemEntryOverlapHandle
{
    MMAP_ENTRY_OVERLAP_HANDLE_OK = 0,
    MMAP_ENTRY_OVERLAP_HANDLE_INVALID = 1
} MemEntryOverlapHandle;

/* 
 * Tries to fix overlaping areas, modifying entry1 in such a way that 
 * entry2 fits. If needed entry1 will be split into multiple areas.
 * Only work if entry1 is of type MMAP_ENTRY_AVAILABLE.
 * @return See MemEntryOverlapHandle enum.
 */
static int mmap_overlap_handle(
    MemoryMapEntry *entry1,
    const MemoryMapEntry *entry2
)
{
    uint8_t overlap = mmap_entry_overlap(entry2, entry1);
    if(overlap == MMAP_ENTRY_OVERLAP_NONE)
        return MMAP_ENTRY_OVERLAP_HANDLE_OK;
    
    if(entry1->type != MMAP_ENTRY_AVAILABLE)
        return MMAP_ENTRY_OVERLAP_HANDLE_INVALID;

    if(entry1->type == entry2->type)
    {
        //todo merge them both into one area.
        return MMAP_ENTRY_OVERLAP_HANDLE_INVALID;
    }

    switch(overlap)
    {
    case MMAP_ENTRY_OVERLAP_START:
        if(mmap_entry_shrink(entry1, (entry2->base + entry2->size - entry1->base)) == 0)
            entry1->base += (entry2->base + entry2->size - entry1->base);
        break;

    case MMAP_ENTRY_OVERLAP_END:
        mmap_entry_shrink(entry1, (entry2->base + entry2->size - entry1->base));
        break;

    case MMAP_ENTRY_OVERLAP_COMPLETE:
    {

        uint64_t init_size = entry1->size;
        entry1->size = entry2->base - entry1->base;
        /* first area needs to be split into two */
        mmap_enlarge(1);
        MemoryMapEntry *newarea = &mmap.entries[mmap.entries_cnt - 1];
        newarea->type = MMAP_ENTRY_AVAILABLE;
        newarea->base = entry2->base + entry2->size;
        newarea->size = init_size - (entry2->base + entry2->size);
        break;
    }
    }

    return MMAP_ENTRY_OVERLAP_HANDLE_OK;
}

static _ATTR_ALWAYS_INLINE int mmap_is_addr_inside_entry(
    uint64_t addr,
    const MemoryMapEntry *entry
)
{
    return (addr > entry->base && addr < entry->base + entry->size);
}

static int __log_fatal_overlap_and_die(
    const MemoryMapEntry *entry1,
    const MemoryMapEntry *entry2
)
{
    kprintf(
        "Refusing to fix overlap for (base 0x%lX size 0x%lX type %ld) (base 0x%lX size 0x%lX type %ld)\n",
        (uint32_t)entry1->base,
        (uint32_t)entry1->size,
        entry1->type,
        (uint32_t)entry2->base,
        (uint32_t)entry2->size,
        entry2->type
    );

    abandon_ship("Illegal memory operation.\n");
    return 0;
}

static void mmap_entries_overlap_handle()
{
    for(size_t i = 0; i < mmap.entries_cnt; ++i)
    {
        MemoryMapEntry *entry1 = &mmap.entries[i];
        for(size_t k = 0; k < mmap.entries_cnt; ++k)
        {
            MemoryMapEntry *entry2 = &mmap.entries[k];
            if(
                entry1 != entry2 &&
                mmap_overlap_handle(entry1, entry2) == MMAP_ENTRY_OVERLAP_HANDLE_INVALID &&
                mmap_overlap_handle(entry2, entry1) == MMAP_ENTRY_OVERLAP_HANDLE_INVALID
            )
            {
                __log_fatal_overlap_and_die(entry1, entry2);
            }
        }
    }
}

void mmap_entry_add(
    uint64_t base,
    uint64_t size,
    uint32_t type
)
{
    /* ignore 64 bit values */
    if(bw_is64_wide(base) || bw_is64_wide(size))
        return;

    mmap_enlarge(1);
    
    MemoryMapEntry *entry = &mmap.entries[mmap.entries_cnt - 1];
    entry->base = base;
    entry->size = size;
    entry->type = type;

    mmap_entries_overlap_handle();
}

int mmap_area_mark_kreserved(
    uint64_t base,
    uint64_t size
)
{
    MemoryMapEntry kreserved;
    kreserved.base = base;
    kreserved.size = size;
    kreserved.type = MMAP_ENTRY_KRESERVED;

    for(size_t i = 0; i < mmap.entries_cnt; ++i)
    {
        MemoryMapEntry *tmp = &mmap.entries[i];
        if(
            mmap_overlap_handle(tmp, &kreserved) == MMAP_ENTRY_OVERLAP_HANDLE_INVALID
        )
        {
            __log_fatal_overlap_and_die(tmp, &kreserved);
        }
    }

    mmap_enlarge(1);
    mmap.entries[mmap.entries_cnt - 1] = kreserved;

    return 0;
}

void mmap_entries_align(
    uint32_t bytes
)
{
    for(size_t i = 0; i < mmap.entries_cnt; ++i)
    {
        MemoryMapEntry *tmp = &mmap.entries[i];
        if(tmp->type == MMAP_ENTRY_AVAILABLE)
        {
            uint64_t aligned_base = (tmp->base + (bytes - 1)) & ~(bytes - 1);
            if(mmap_is_addr_inside_entry(aligned_base, tmp))
            {
                mmap_entry_shrink(tmp, aligned_base - tmp->base);
                tmp->base = aligned_base;
            }
        }
    }
}

const MemoryMap *mmap_get_mmap()
{
    return &mmap;
}

void mmap_print()
{
    kprintf("System memory map:\n");
    for(size_t i = 0; i < mmap.entries_cnt; ++i)
    {
        const MemoryMapEntry *tmp = &mmap.entries[i];
        kprintf(
            "base: 0x%lX size: 0x%lX type: %ld\n", 
            (uint32_t)tmp->base, 
            (uint32_t)tmp->size, 
            tmp->type
        );
    }
}
