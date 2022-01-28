/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/kmalloc.h>
#include<dxgmx/string.h>
#include<dxgmx/mem/kheap.h>
#include<dxgmx/utils/bitwise.h>
#include<dxgmx/assert.h>
#include<dxgmx/math.h>
#include<dxgmx/klog.h>

#if defined(_X86_)
#   define KMALLOC_SLAB_SIZE 32
#elif defined(_X86_64_)
#   define KMALLOC_SLAB_SIZE 64
#endif // defined(_X86_)

#define KLOGF(lvl, fmt, ...) klogln(lvl, "kmalloc: " fmt, ##__VA_ARGS__)

#define KMALLOC_SLABS_PER_BLOCK 64
#define KMALLOC_BLOCKS_COUNT (KERNEL_HEAP_SIZE / KMALLOC_SLABS_PER_BLOCK / KMALLOC_SLAB_SIZE)

static u64 g_allocator_blocks[KMALLOC_BLOCKS_COUNT];

_ATTR_ALWAYS_INLINE static ptr vaddr_from_block_and_slab(
    size_t block, 
    size_t slab_offset
)
{
    return kheap_get_start_vaddr() + 
        block * KMALLOC_SLABS_PER_BLOCK * KMALLOC_SLAB_SIZE + 
        slab_offset * KMALLOC_SLAB_SIZE;
}

_ATTR_ALWAYS_INLINE static size_t block_from_vaddr(
    ptr vaddr
)
{
    return (vaddr - kheap_get_start_vaddr()) / (KMALLOC_SLABS_PER_BLOCK * KMALLOC_SLAB_SIZE);
}

static i32 find_contiguous_blocks_aligned(
    size_t start, 
    size_t blocks, 
    size_t alignment
)
{
    /* Make sure the start block is aligned. */
    ptr addr = vaddr_from_block_and_slab(start, 0);
    addr = (addr + alignment - 1) & ~(alignment - 1);
    start = block_from_vaddr(addr);

    if(start >= KMALLOC_BLOCKS_COUNT || !blocks)
        return -1;

    size_t block_stepping = ceil(
        (float)alignment / (KMALLOC_BLOCKS_COUNT * KMALLOC_SLABS_PER_BLOCK)
    );

    for(size_t i = start; i < KMALLOC_BLOCKS_COUNT; i += block_stepping)
    {
        size_t found = 0;
        for(size_t k = 0; k < blocks && i + k < KMALLOC_BLOCKS_COUNT; ++k)
        {
            if(g_allocator_blocks[i + k])
                break;
            else
                ++found;
        }

        if(found == blocks)
            return i;
    }

    return -1;
}

bool kmalloc_init()
{
    memset(g_allocator_blocks, 0, sizeof(g_allocator_blocks));
    return true;
}

void *kmalloc(size_t size)
{
    return kmalloc_aligned(size, 1);
}

void *kmalloc_aligned(size_t size, size_t alignment)
{
    if(!size || (!alignment || !bw_is_power_of_two(alignment)))
        return NULL;

    size_t slabs = ceil((float)size / KMALLOC_SLAB_SIZE);
    size_t blocks = slabs / KMALLOC_SLABS_PER_BLOCK;

    u64 slabs_mask = 0;
    i32 start_block = 0;
    size_t slabs_offset = 0;

    if(blocks)
    {
        start_block = find_contiguous_blocks_aligned(0, blocks, alignment);
        if(start_block < 0)
        {
            KLOGF(WARN, "Failed to find %u contiguous bytes.", size);
            return NULL;
        }

        slabs -= blocks * KMALLOC_SLABS_PER_BLOCK;
        if(!slabs)
            goto done; /* how nice. */

        /* build a mask for the new 'half' block */
        for(size_t i = 0; i < slabs; ++i)
            bw_set(&slabs_mask, i);

        u64 half_block = g_allocator_blocks[start_block + blocks];
        while(
            (half_block & slabs_mask) || 
            ((half_block >> 32) & (slabs_mask >> 32))
        )
        {
            /* Try to find a new start_block. If this fails at any point, it means we can't make
            this allocation as we reached the end of the heap without finding anything. */
            start_block = find_contiguous_blocks_aligned(start_block + 1, blocks, alignment);
            if(start_block < 0)
            {
                KLOGF(WARN, "Failed to find %u contiguous bytes.", size);
                return NULL;
            }
            half_block = g_allocator_blocks[start_block + blocks];
        }
    }
    else
    {
        for(size_t i = 0; i < KMALLOC_BLOCKS_COUNT; ++i)
        {
            start_block = i;
            u64 block = g_allocator_blocks[i];

            slabs_offset = 0;
            slabs_mask = 0;
            for(size_t i = 0; i < slabs; ++i)
                bw_set(&slabs_mask, i);

            for(size_t k = 0; k < KMALLOC_SLABS_PER_BLOCK - slabs; ++k)
            {
                if(
                    (block & slabs_mask) == 0 && 
                    ((block >> 32) & (slabs_mask >> 32)) == 0
                )
                {
                    goto done;
                }
                slabs_mask <<= 1;
                ++slabs_offset;
            }
        }

        KLOGF(WARN, "Failed to find %u contiguous bytes.", size);
        return NULL;
    }

done:
    /* mark any allocations made. */
    for(size_t i = 0; i < blocks; ++i)
    {
        u64 *block = &g_allocator_blocks[start_block + i];
        *block |= 0xFFFFFFFF;
        *block <<= 32;
        *block |= 0xFFFFFFFF;
    }

    if(slabs_mask)
    {
        u64 *block = &g_allocator_blocks[start_block + blocks];
        u32 old = *block;
        *block >>= 32;
        *block |= (slabs_mask >> 32);
        *block <<= 32;
        *block |= slabs_mask & 0xFFFFFFFF;
        *block |= old;
    }

    ptr addr = vaddr_from_block_and_slab(start_block, slabs_offset);
    
    KLOGF(DEBUG, "Allocated %u bytes at %p-%p.", size, (void *)addr, (void *)(addr + size));
    return (void *)addr;
}

void kfree(void *addr)
{
    ASSERT(
        (ptr)addr % KMALLOC_SLAB_SIZE == 0 &&
        ((ptr)addr >= kheap_get_start_vaddr() && (ptr)addr < kheap_get_start_vaddr() + kheap_get_size())
    );

    /* dang */
}

void *krealloc(void *addr, size_t size)
{
    (void)addr;
    (void)size;
    return NULL;
}
