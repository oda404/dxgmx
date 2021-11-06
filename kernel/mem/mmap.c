/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/mmap.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/compiler_attrs.h>
#include<stddef.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "mmap: " fmt, ##__VA_ARGS__);

static const char *mmap_entry_type_to_str(u8 type)
{
    switch(type)
    {
    case MMAP_AVAILABLE:
        return "available";
    case MMAP_RESERVED:
        return "reserved";
    case MMAP_ACPI_RECLAIMABLE:
        return "ACPI reclaimable";
    case MMAP_NVS:
        return "ACPI NVS";
    case MMAP_BADRAM:
        return "badram";
    default:
        return "???";
    }
}

static int mmap_rm_entry_at_idx(size_t idx, MemoryMap *mmap)
{
    if(idx >= mmap->entries_cnt)
        return 1;

    for(size_t i = idx; i < mmap->entries_cnt - 1; ++i)
    {
        mmap->entries[i] = mmap->entries[i + 1];
    }
    --mmap->entries_cnt;
    return 0;
}

/* 
 * Tries to shrink the mem entry by 'size' bytes.
 * If after shrinking, the area's size <= 0, it will be removed.
 * @param area Memory entry
 * @param size How much to shrink by.
 * @return 1 if area was removed, 0 if it was only shrunk.
 */
static int mmap_shrink_entry(
    size_t idx, 
    u64 size,
    MemoryMap *mmap
)
{
    MemRangeTyped *tmp = &mmap->entries[idx];

    if(size >= tmp->size)
    {
        mmap_rm_entry_at_idx(idx, mmap);
        return 1;
    }

    tmp->size -= size;
    return 0;
}

static int mmap_enlarge(size_t n, MemoryMap *mmap)
{
    if(mmap->entries_cnt + n > MMAP_MAX_ENTRIES_CNT)
        abandon_ship("Exceeded MMAP_MAX_ENTRIES_CNT when enlarging mmap");
    mmap->entries_cnt += n;
    return n;
}

static const MemRangeTyped *mmap_get_overlapping_entry(
    u64 base,
    u64 size,
    MemoryMap *mmap
)
{
    for(size_t i = 0; i < mmap->entries_cnt; ++i)
    {
        const MemRangeTyped *tmp = &mmap->entries[i];
        if(
            (base > tmp->base && (base + size < tmp->base + tmp->size || base < tmp->base + tmp->size)) ||
            (base <= tmp->base && (base + size >= tmp->base + tmp->size || base + size > tmp->base))
        )
        {
            return tmp;
        }
    }

    return NULL;
}

static void mmap_fix_new_overlaps(const MemRangeTyped *e, MemoryMap *mmap)
{
    for(size_t i = 0; i < mmap->entries_cnt; ++i)
    {
        MemRangeTyped *tmp = &mmap->entries[i];
        if(e->base > tmp->base)
        {
            if(e->base + e->size < tmp->base + tmp->size)
            {
                /** The new entry falls inside a seconds entry named 'tmp', 
                 * so 'tmp' is split into two and the new entry showed in between. 
                 * +-----------------+     +---++===++---+
                 * |     +========+  |     |   ||   ||   |
                 * | tmp |  e     |  | ->  |tmp|| e ||tmp|
                 * |     +========+  |     | 1 ||   || 2 |
                 * +-----------------+     +---++===++---+
                */
                u64 prevbase = tmp->base;
                tmp->base = e->base + e->size;
                tmp->size -= tmp->base;
                mmap_add_entry(prevbase, e->base - prevbase, tmp->type, mmap);
            }
            else if(e->base < tmp->base + tmp->size)
            {
                /** The new entry overlaps a second entry named 'tmp' 
                 * at it's end, so we shrink 'tmp'.
                 *        +==========+    +---++=====+
                 * +------|+         |    |   ||     |
                 * | tmp  ||    e    | -> |tmp||  e  |
                 * +------|+         |    |   ||     |
                 *        +==========+    +---++=====+
                */
                mmap_shrink_entry(i, tmp->base + tmp->size - e->base, mmap);
            }
        }
        else
        {
            if(e->base + e->size >= tmp->base + tmp->size)
            {
                /** The new entry completely overlaps a second 
                 * entry named 'tmp', so we remove 'tmp'.
                 * +---------------+     +---------------+
                 * |   +========+  |     |               |
                 * | e |  tmp   |  |  -> |       e       |
                 * |   +========+  |     |               |
                 * +---------------+     +---------------+
                */
                mmap_rm_entry_at_idx(i, mmap);
            }
            else if(e->base + e->size > tmp->base)
            {
                /** The new entry overlaps another entry named 'tmp' at 
                 * it's begining, so we shrink 'tmp' and move it's base.
                 * +==========+            +=======++---+
                 * |         +|-------+    |       ||   |
                 * |    e    ||  tmp  | -> |   e   ||tmp|
                 * |         +|-------+    |       ||   |
                 * +==========+            +=======++---+
                */
                if(mmap_shrink_entry(i, e->base + e->size - tmp->base, mmap) == 0)
                    tmp->base += e->base + e->size - tmp->base;
            }
        }
    }
}

void mmap_init(MemoryMap *mmap)
{
    mmap->entries_cnt = 0;
}

void mmap_add_entry(
    u64 base,
    u64 size,
    u8 type,
    MemoryMap *mmap
)
{
    if(bw_is64_wide(base) || bw_is64_wide(size))
    {
        klog(INFO, "ass\n");
        return;
    }

    MemRangeTyped e = {
        .base = base,
        .size = size,
        .type = type
    };

    /** When a new entry is added it is checked to see if it overlaps
     * with any other entries. If overlaps are found the newly added entry
     * will NOT be modified, instead the overlapping area(s) will, even if their
     * types aren't MMAP_AVAILABLE.
    */
    mmap_fix_new_overlaps(&e, mmap);

    mmap_enlarge(1, mmap);
    mmap->entries[mmap->entries_cnt - 1] = e;
    
    /* sort mmap based on the bases */
    for(size_t i = 0; i < mmap->entries_cnt; ++i)
    {
        for(size_t k = 0; k < mmap->entries_cnt - 1; ++k)
        {
            if(mmap->entries[k].base > mmap->entries[k + 1].base)
            {
                //FIXME
                MemRangeTyped tmp = mmap->entries[k];
                mmap->entries[k] = mmap->entries[k + 1];
                mmap->entries[k + 1] = tmp;
            }
        }
    }
}

void mmap_align_entries(u8 type, u32 align, MemoryMap *mmap)
{
    for(size_t i = 0; i < mmap->entries_cnt; ++i)
    {
        MemRangeTyped *tmp = &mmap->entries[i];
        if(tmp->type == type)
        {
            u64 aligned_base = (tmp->base + (align - 1)) & ~(align - 1);
            if(mmap_is_addr_inside_entry(aligned_base, tmp))
            {
                mmap_shrink_entry(i, aligned_base - tmp->base, mmap);
                tmp->base = aligned_base;
            }
        }
    }
}

int mmap_update_entry_type(
    u64 base,
    u64 size,
    u8 type,
    MemoryMap *mmap
)
{
    const MemRangeTyped *overlap = mmap_get_overlapping_entry(
        base, size, mmap
    );

    if(overlap && type != overlap->type)
    {
        KLOGF(
            INFO,
            "updating [mem 0x%p-0x%p] %s -> %s.\n",
            (void *)(ptr)base, 
            (void *)(ptr)(base + size - 1),
            mmap_entry_type_to_str(overlap->type),
            mmap_entry_type_to_str(type)
        );
        mmap_add_entry(base, size, type, mmap);

        return 0;
    }

    return 1;
}

void mmap_dump(const MemoryMap *mmap)
{
    for(size_t i = 0; i < mmap->entries_cnt; ++i)
    {
        const MemRangeTyped *tmp = &mmap->entries[i];
        KLOGF(
            INFO,
            "[mem 0x%p-0x%p] %s.\n",
            (void *)(ptr)tmp->base, 
            (void *)(ptr)(tmp->base + tmp->size - 1),
            mmap_entry_type_to_str(tmp->type)
        );
    }
}

_ATTR_ALWAYS_INLINE bool 
mmap_is_addr_inside_entry(u64 addr, const MemRangeTyped *entry)
{
    return (addr >= entry->base && addr < entry->base + entry->size);
}
