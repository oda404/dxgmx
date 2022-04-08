/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/kheap.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bitwise.h>
#include <dxgmx/utils/hashtable.h>

#if defined(_X86_)
#define KMALLOC_SLAB_SIZE 32
#define KMALLOC_SLABS_PER_BLOCK 32
#elif defined(_X86_64_)
#define KMALLOC_SLAB_SIZE 64
#define KMALLOC_SLABS_PER_BLOCK 64
#endif // defined(_X86_)

#define KLOGF(lvl, fmt, ...)                                                   \
    klogln(lvl, "kmalloc: " fmt __VA_OPT__(, ) __VA_ARGS__)

#define KMALLOC_BLOCKS_COUNT                                                   \
    (KERNEL_HEAP_SIZE / KMALLOC_SLABS_PER_BLOCK / KMALLOC_SLAB_SIZE)

#define KMALLOC_LOG_ACTIONS 0

static HashTable g_allocation_hashtable;
static u64 g_allocator_blocks[KMALLOC_BLOCKS_COUNT];

static _ATTR_ALWAYS_INLINE bool is_vaddr_inside_heap(ptr vaddr)
{
    return (
        (ptr)vaddr >= kheap_get_start_vaddr() &&
        (ptr)vaddr < kheap_get_start_vaddr() + kheap_get_size());
}

static _ATTR_ALWAYS_INLINE ptr
vaddr_from_block_and_slab(size_t block, size_t slab_offset)
{
    return kheap_get_start_vaddr() +
           block * KMALLOC_SLABS_PER_BLOCK * KMALLOC_SLAB_SIZE +
           slab_offset * KMALLOC_SLAB_SIZE;
}

static _ATTR_ALWAYS_INLINE size_t block_from_vaddr(ptr vaddr)
{
    return (vaddr - kheap_get_start_vaddr()) /
           (KMALLOC_SLABS_PER_BLOCK * KMALLOC_SLAB_SIZE);
}

static i32
find_contiguous_blocks_aligned(size_t start, size_t blocks, size_t alignment)
{
    /* Make sure the start block is aligned. */
    ptr addr = vaddr_from_block_and_slab(start, 0);
    addr = (addr + alignment - 1) & ~(alignment - 1);
    start = block_from_vaddr(addr);

    if (start >= KMALLOC_BLOCKS_COUNT || !blocks)
        return -1;

    size_t block_stepping =
        ceil((double)alignment / (KMALLOC_SLABS_PER_BLOCK * KMALLOC_SLAB_SIZE));

    for (size_t i = start; i < KMALLOC_BLOCKS_COUNT; i += block_stepping)
    {
        size_t found = 0;
        for (size_t k = 0; k < blocks && i + k < KMALLOC_BLOCKS_COUNT; ++k)
        {
            if (g_allocator_blocks[i + k])
                break;
            else
                ++found;
        }

        if (found == blocks)
            return i;
    }

    return -1;
}

bool kmalloc_init()
{
    memset(g_allocator_blocks, 0, sizeof(g_allocator_blocks));

    /* FIXME: This creates a circular dependency where
    the hashtable needs kmalloc to be in working order and
    kmalloc needs the hashtable to keep track of all allocation sizes. */
    hashtable_init(
        (KMALLOC_SLAB_SIZE * KMALLOC_SLABS_PER_BLOCK) / sizeof(HashTableEntry),
        &g_allocation_hashtable);

    return true;
}

void* kmalloc(size_t size)
{
    return kmalloc_aligned(size, 4);
}

void* kmalloc_aligned(size_t size, size_t alignment)
{
    if (!size || !bw_is_power_of_two(alignment))
        return NULL;

    size_t slabs = ceil((double)size / KMALLOC_SLAB_SIZE);
    size_t blocks = slabs / KMALLOC_SLABS_PER_BLOCK;

    u64 slabs_mask = 0;
    i32 start_block = 0;

    if (blocks)
    {
        start_block = find_contiguous_blocks_aligned(0, blocks, alignment);
        if (start_block < 0)
        {
            KLOGF(WARN, "Failed to find %u contiguous bytes.", size);
            return NULL;
        }

        slabs -= blocks * KMALLOC_SLABS_PER_BLOCK;
        if (!slabs)
            goto done; /* how nice. */

        /* build a mask for the new 'half' block */
        for (size_t i = 0; i < slabs; ++i)
            bw_set(&slabs_mask, i);

        u64 block = g_allocator_blocks[start_block + blocks];
        while (bw_mask(block, slabs_mask))
        {
            /* Try to find a new start_block. If this fails at any point, it
            means we can't make this allocation as we reached the end of the
            heap without finding anything. */
            start_block = find_contiguous_blocks_aligned(
                start_block + 1, blocks, alignment);
            if (start_block < 0)
            {
                KLOGF(WARN, "Failed to find %u contiguous bytes.", size);
                return NULL;
            }
            block = g_allocator_blocks[start_block + blocks];
        }
    }
    else
    {
        for (size_t i = 0; i < KMALLOC_BLOCKS_COUNT; ++i)
        {
            start_block = i;
            u64 block = g_allocator_blocks[i];

            slabs_mask = 0;
            for (size_t k = 0; k < slabs; ++k)
                bw_set(&slabs_mask, k);

            size_t k = 0;
            for (; k < KMALLOC_SLABS_PER_BLOCK - slabs; ++k)
            {
                if (!bw_mask(block, slabs_mask))
                    goto done;
                slabs_mask <<= 1;
            }
        }

        KLOGF(WARN, "Failed to find %u contiguous bytes.", size);
        return NULL;
    }

done:
    /* mark any whole blocks. */
    for (size_t i = 0; i < blocks; ++i)
        bw_or_mask(&g_allocator_blocks[start_block + i], 0xFFFF'FFFF'FFFF'FFFF);
    /* apply slabs_mask if  */
    if (slabs_mask)
        bw_or_mask(&g_allocator_blocks[start_block + blocks], slabs_mask);

    size_t start_slab = 0;
    if (slabs_mask)
    {
        while (!((slabs_mask >> start_slab) & 1))
            ++start_slab;
    }

    const ptr addr = vaddr_from_block_and_slab(start_block, start_slab);

    if (g_allocation_hashtable.entries_max)
    {
        // FIXME: Remove this shit.
        if (!hashtable_add(addr, (void*)size, &g_allocation_hashtable))
            panic("kmalloc: Allocation hashtable reached it's capacity.");
    }

#if KMALLOC_LOG_ACTIONS
    KLOGF(
        DEBUG,
        "Allocated %u(%u) bytes at 0x%p.",
        size,
        KMALLOC_SLAB_SIZE * slabs +
            KMALLOC_SLAB_SIZE * KMALLOC_SLABS_PER_BLOCK * blocks,
        (void*)addr);
#endif // KMALLOC_LOG_ACTIONS

    return (void*)addr;
}

void kfree(void* addr)
{
    if ((ptr)addr % KMALLOC_SLAB_SIZE || !is_vaddr_inside_heap((ptr)addr))
        panic("Tried to kfree() an invalid address!");

    const size_t size =
        (size_t)hashtable_get((size_t)addr, &g_allocation_hashtable);
    if (!size)
        panic("Tried to kfree() an invalid address!");

    size_t slabs = ceil((double)size / KMALLOC_SLAB_SIZE);
    size_t blocks = slabs / KMALLOC_SLABS_PER_BLOCK;
    slabs -= blocks * KMALLOC_SLABS_PER_BLOCK;

    size_t start_block = block_from_vaddr((ptr)addr);
    for (size_t i = 0; i < blocks; ++i)
        g_allocator_blocks[start_block + i] = 0;

    if (slabs)
    {
        /* build the bitmask for the slabs */
        size_t offset =
            ((ptr)addr % (KMALLOC_SLAB_SIZE * KMALLOC_SLABS_PER_BLOCK)) /
            KMALLOC_SLAB_SIZE;
        u64 mask = 0xFFFF'FFFF'FFFF'FFFF;
        for (size_t i = 0; i < slabs; ++i)
            bw_clear(&mask, i + offset);

        bw_and_mask(&g_allocator_blocks[start_block + blocks], mask);
    }

    hashtable_remove((size_t)addr, &g_allocation_hashtable);

#if KMALLOC_LOG_ACTIONS
    KLOGF(
        DEBUG,
        "Freed %u(%u) bytes at 0x%p.",
        size,
        KMALLOC_SLAB_SIZE * slabs +
            KMALLOC_SLAB_SIZE * KMALLOC_SLABS_PER_BLOCK * blocks,
        addr);
#endif // KMALLOC_LOG_ACTIONS
}

void* krealloc(void* addr, size_t size)
{
    void* newaddr = NULL;
    if (addr)
    {
        const size_t oldsize =
            (size_t)hashtable_get((size_t)addr, &g_allocation_hashtable);
        kfree(addr);
        newaddr = kmalloc(size);
        memcpy(newaddr, addr, oldsize);
    }
    else
    {
        newaddr = kmalloc(size);
    }

    return newaddr;
}
