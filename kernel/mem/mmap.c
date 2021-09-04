/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/mmap.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/attrs.h>
#include<stddef.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "mmap: " fmt, ##__VA_ARGS__);

static MemoryMap g_mmap;

void mmap_init()
{
    g_mmap.entries_cnt = 0;
}

static const char *mmap_entry_type_to_str(u32 type)
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

static int mmap_rm_entry(MemoryMapEntry *area)
{
    for(size_t i = 0; i < g_mmap.entries_cnt; ++i)
    {
        if(&g_mmap.entries[i] == area)
        {
            for(; i < g_mmap.entries_cnt - 1; ++i)
            {
                g_mmap.entries[i] = g_mmap.entries[i + 1];
            }
            --g_mmap.entries_cnt;
            return 0;
        }
    }
    return 1;
}

/* 
 * Tries to shrink the mem entry by 'size' bytes.
 * If after shrinking, the area's size <= 0, it will be removed.
 * @param area Memory entry
 * @param size How much to shrink by.
 * @return 1 if area was removed, 0 if it was only shrunk.
 */
static int mmap_shrink_entry(
    MemoryMapEntry *entry, 
    u64 size
)
{
    if(size >= entry->size)
    {
        mmap_rm_entry(entry);
        return 1;
    }

    entry->size -= size;
    return 0;
}

static int mmap_enlarge(size_t n)
{
    if(g_mmap.entries_cnt + n > MMAP_MAX_ENTRIES_CNT)
        abandon_ship("Exceeded MMAP_MAX_ENTRIES_CNT when enlarging g_mmap.");
    g_mmap.entries_cnt += n;
    return n;
}

static const MemoryMapEntry *mmap_get_overlapping_entry(
    u64 base,
    u64 size
)
{
    for(size_t i = 0; i < g_mmap.entries_cnt; ++i)
    {
        const MemoryMapEntry *tmp = &g_mmap.entries[i];
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

static void mmap_fix_new_overlaps(const MemoryMapEntry *e)
{
    for(size_t i = 0; i < g_mmap.entries_cnt; ++i)
    {
        MemoryMapEntry *tmp = &g_mmap.entries[i];
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
                mmap_add_entry(prevbase, e->base - prevbase, tmp->type);
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
                mmap_shrink_entry(tmp, tmp->base + tmp->size - e->base);
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
                mmap_rm_entry(tmp);
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
                if(mmap_shrink_entry(tmp, e->base + e->size - tmp->base) == 0)
                    tmp->base += e->base + e->size - tmp->base;
            }
        }
    }
}

static _ATTR_ALWAYS_INLINE int mmap_is_addr_inside_entry(
    uint64_t addr,
    const MemoryMapEntry *entry
)
{
    return (addr > entry->base && addr < entry->base + entry->size);
}

void mmap_add_entry(
    u64 base,
    u64 size,
    u32 type
)
{
    /* ignore 64 bit values */
    if(bw_is64_wide(base) || bw_is64_wide(size))
        return;

    MemoryMapEntry e = {
        .base = base,
        .size = size,
        .type = type
    };

    /** When a new entry is added it is checked to see if it overlaps
     * with any other entries. If overlaps are found the newly added entry
     * will NOT be modified, instead the overlapping area(s) will, even if their
     * types aren't MMAP_AVAILABLE.
    */
    mmap_fix_new_overlaps(&e);

    mmap_enlarge(1);
    g_mmap.entries[g_mmap.entries_cnt - 1] = e;
    
    /* sort mmap based on the bases */
    for(size_t i = 0; i < g_mmap.entries_cnt; ++i)
    {
        for(size_t k = 0; k < g_mmap.entries_cnt - 1; ++k)
        {
            if(g_mmap.entries[k].base > g_mmap.entries[k + 1].base)
            {
                //FIXME
                MemoryMapEntry tmp = g_mmap.entries[k];
                g_mmap.entries[k] = g_mmap.entries[k + 1];
                g_mmap.entries[k + 1] = tmp;
            }
        }
    }
}

void mmap_align_entries(
    uint32_t bytes
)
{
    for(size_t i = 0; i < g_mmap.entries_cnt; ++i)
    {
        MemoryMapEntry *tmp = &g_mmap.entries[i];
        if(tmp->type == MMAP_AVAILABLE)
        {
            ptr aligned_base = (tmp->base + (bytes - 1)) & ~(bytes - 1);
            if(mmap_is_addr_inside_entry(aligned_base, tmp))
            {
                mmap_shrink_entry(tmp, aligned_base - tmp->base);
                tmp->base = aligned_base;
            }
        }
    }
}

int mmap_update_entry_type(
    u64 base,
    u64 size,
    u32 type
)
{
    const MemoryMapEntry *overlap = mmap_get_overlapping_entry(
        base, size
    );

    if(overlap && type != overlap->type)
    {
        KLOGF(
            KLOG_INFO,
            "updating [mem 0x%0*lX-0x%0*lX] %s -> %s.\n",
            sizeof(ptr) == 4 ? 8 : 16, 
            (ptr)base, 
            sizeof(ptr) == 4 ? 8 : 16,
            /* FIXME klog doesn't yet support 64 bit numbers but 
            tmp->base + tmp->size might be 64 bits wide even in 32 bit mode. */
            (ptr)(base + size),
            mmap_entry_type_to_str(overlap->type),
            mmap_entry_type_to_str(type)
        );
        mmap_add_entry(base, size, type);

        return 0;
    }

    return 1;
}

const MemoryMap *mmap_get_mmap()
{
    return &g_mmap;
}

void mmap_dump()
{
    for(size_t i = 0; i < g_mmap.entries_cnt; ++i)
    {
        const MemoryMapEntry *tmp = &g_mmap.entries[i];
        KLOGF(
            KLOG_INFO,
            "[mem 0x%0*lX-0x%0*lX] %s.\n",
            sizeof(ptr) == 4 ? 8 : 16, 
            (ptr)tmp->base, 
            sizeof(ptr) == 4 ? 8 : 16,
            /* FIXME klog doesn't yet support 64 bit numbers but 
            tmp->base + tmp->size might be 64 bits wide even in 32 bit mode. */
            (ptr)(tmp->base + tmp->size),
            mmap_entry_type_to_str(tmp->type)
        );
    }
}
