/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/bitwise.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "gallocator: "

#define LO_POOL_CHUNK_SIZE 32
#define MID_POOL_CHUNK_SIZE 64
#define HI_POOL_CHUNK_SIZE 128

typedef struct S_GAllocatorHeapMeta
{
#define HEAP_SIGNATURE 0xDEAD5160
    u32 signature;

    ptr lo_bitmap;
    size_t lo_bitmap_size;
    ptr lo_pool;
    ptr lo_poolchunks;

    ptr mid_bitmap;
    size_t mid_bitmap_size;
    ptr mid_pool;
    ptr mid_poolchunks;

    ptr hi_bitmap;
    size_t hi_bitmap_size;
    ptr hi_pool;
    ptr hi_poolchunks;
} GAllocatorHeapMeta;

typedef struct S_GAllocationMeta
{
#define ALLOCATION_SIGNATURE 0xDEADADD5
    u32 signature;
    size_t size;
    size_t chunksize;
    size_t alignment;
} GAllocationMeta;

typedef struct S_AllocationCtx
{
    ptr bitmapstart;
    size_t bitmapsize;

    ptr poolstart;
    size_t poolchunks;

    size_t chunksize;

    ptr byte;
    i8 bit;
} AllocationCtx;

/* lazy hack to make sure the metadata fits no matter the chunk size. We could
 * fix this by just allocating however many chunks we need to fit the
 * metadata... */
STATIC_ASSERT(
    sizeof(GAllocationMeta) <= LO_POOL_CHUNK_SIZE,
    "GAllocationMeta doesn't fit in the smallest chunk!");

static void allocationctx_advance(size_t bits, AllocationCtx* alloc)
{
    while (bits >= 8)
    {
        ++alloc->byte;
        bits -= 8;
    }

    if (alloc->bit + bits >= 8)
    {
        ++alloc->byte;
        alloc->bit = alloc->bit + (bits - 8);
    }
    else
    {
        alloc->bit += bits;
    }
}

static ptr allocationctx_to_offset(const AllocationCtx* alloc)
{
    return alloc->byte * 8 * alloc->chunksize + alloc->bit * alloc->chunksize;
}

static void addr_to_allocationctx_offset(ptr addr, AllocationCtx* alloc)
{
    addr -= alloc->poolstart;
    size_t totalchunks = addr / alloc->chunksize;

    alloc->byte = totalchunks / 8;
    alloc->bit = totalchunks - alloc->byte * 8;
}

static bool is_allocationctx_inside_pool(const AllocationCtx* alloc)
{
    return allocationctx_to_offset(alloc) <
           alloc->poolchunks * alloc->chunksize;
}

static bool check_bit(const AllocationCtx* alloc)
{
    u8 byte = ((u8*)alloc->bitmapstart)[alloc->byte];
    return byte & (1 << alloc->bit);
}

static size_t gallocator_has_n_contiguous_bits(AllocationCtx alloc, size_t n)
{
    size_t foundbits = 0;

    /* These should both be true as this function is called with a valid
     * starting point. */
    ASSERT(alloc.bit < 8);
    ASSERT(is_allocationctx_inside_pool(&alloc));

    while (alloc.byte < alloc.bitmapsize)
    {
        u8 byte = ((u8*)alloc.bitmapstart)[alloc.byte];

        while (is_allocationctx_inside_pool(&alloc))
        {
            if ((byte & (1 << alloc.bit)) > 0)
                goto done;

            ++foundbits;
            if (foundbits == n)
                goto done;

            if (alloc.bit == 7)
            {
                alloc.bit = 0;
                break;
            }
            ++alloc.bit;
        }
        ++alloc.byte;
    }

done:
    /* We should find at least 1 bit since again, this function is called with
     * a valid starting point */
    ASSERT(foundbits > 0);
    return foundbits;
}

static int gallocator_bitmap_set_bits(AllocationCtx alloc, size_t bits)
{
    /* This function is called with a valid starting point. The following
     * asserts are there as a sanity check. */
    if (alloc.bit > 7 || !is_allocationctx_inside_pool(&alloc))
        return -EINVAL;

    {
        AllocationCtx tmpalloc = alloc;
        allocationctx_advance(bits, &tmpalloc);
        if (!is_allocationctx_inside_pool(&tmpalloc))
            return -EINVAL;
    }

    /* Mark the metadata chunk */
    if (alloc.bit == 0)
    {
        if (alloc.byte == 0)
            return 1; // There is no metadata chunk :(

        ((u8*)alloc.bitmapstart)[alloc.byte - 1] |= (1 << 7);
    }
    else
    {
        ((u8*)alloc.bitmapstart)[alloc.byte] |= (1 << (alloc.bit - 1));
    }

    // Mark the actual allocation
    while (alloc.byte < alloc.bitmapsize)
    {
        u8* byte = &((u8*)alloc.bitmapstart)[alloc.byte];
        while (is_allocationctx_inside_pool(&alloc))
        {
            *byte |= (1 << alloc.bit);
            --bits;

            if (!bits)
                return 0;

            if (alloc.bit == 7)
            {
                alloc.bit = 0;
                break;
            }
            ++alloc.bit;
        }
        ++alloc.byte;
    }

    ASSERT_NOT_HIT();
}

static int gallocator_bitmap_clear_bits(AllocationCtx alloc, size_t bits)
{
    /* This function is called with a valid starting point. The following
     * asserts are there as a sanity check. */
    if (alloc.bit > 7 || !is_allocationctx_inside_pool(&alloc))
        return -EINVAL;

    {
        AllocationCtx tmpalloc = alloc;
        allocationctx_advance(bits, &tmpalloc);
        if (!is_allocationctx_inside_pool(&tmpalloc))
            return -EINVAL;
    }

    /* Mark the metadata chunk */
    if (alloc.bit == 0)
    {
        if (alloc.byte == 0)
            return 1; // There is no metadata chunk :(

        ((u8*)alloc.bitmapstart)[alloc.byte - 1] &= ~(1 << 7);
    }
    else
    {
        ((u8*)alloc.bitmapstart)[alloc.byte] &= ~(1 << (alloc.bit - 1));
    }

    // Mark the actual allocation
    while (alloc.byte < alloc.bitmapsize)
    {
        u8* byte = &((u8*)alloc.bitmapstart)[alloc.byte];
        while (is_allocationctx_inside_pool(&alloc))
        {
            *byte &= ~(1 << alloc.bit);
            --bits;

            if (!bits)
                return 0;

            if (alloc.bit == 7)
            {
                alloc.bit = 0;
                break;
            }
            ++alloc.bit;
        }
        ++alloc.byte;
    }

    ASSERT_NOT_HIT();
}

static bool
gallocator_find_start_aligned(size_t alignment, AllocationCtx* alloc)
{
    ptr starting_address = alloc->poolstart + allocationctx_to_offset(alloc);

    /* Find an aligned address */
    if (starting_address % alignment)
    {
        starting_address = bytes_align_up64(starting_address, alignment);
        ASSERT((starting_address % alignment) == 0); // sanity check

        addr_to_allocationctx_offset(starting_address, alloc);
    }

    bool found = false;
    while (alloc->byte < alloc->bitmapsize &&
           is_allocationctx_inside_pool(alloc))
    {
        /* Check if the starting chunk is free */
        int ok = check_bit(alloc);

        /* Check if the chunk before that is also free (used for metadata) */
        AllocationCtx tmp = *alloc;
        if (tmp.bit == 0)
        {
            if (tmp.byte == 0)
                goto next;

            tmp.bit = 7;
            --tmp.byte;
        }
        else
        {
            --tmp.bit;
        }

        ok += check_bit(&tmp);

        if (ok == 0)
        {
            found = true;
            break;
        }

    next:
        allocationctx_advance(
            ceil((double)alignment / alloc->chunksize), alloc);
    }

    return found;
}

static void* gallocator_do_alloc_aligned(
    size_t size, size_t alignment, const GAllocatorHeapMeta* meta)
{
    /** Here is a rundown of how this allocator works:
     *  1: We find the optimum chunk size for the given size.
     *
     *  2: We find a suitable (aligned) starting chunk. This starting chunk will
     * also ensure sizeof(GAllocationMeta) size right before it's starting
     * location.
     *
     *  3: We check to see if the above starting chunk actually has our required
     * number of chunks available. If not go back to step 2. *
     *
     *  4: We mark the bitmap corresponding to our chunksize.
     *
     *  5: We put an GAllocationMeta struct right before this newly made
     * allocation.
     */

    // 1:
    AllocationCtx alloc;
    memset(&alloc, 0, sizeof(AllocationCtx));

    if (size >= HI_POOL_CHUNK_SIZE)
    {
        alloc.chunksize = HI_POOL_CHUNK_SIZE;
        alloc.poolstart = meta->hi_pool;
        alloc.poolchunks = meta->hi_poolchunks;
        alloc.bitmapstart = meta->hi_bitmap;
        alloc.bitmapsize = meta->hi_bitmap_size;
    }
    else if (size >= MID_POOL_CHUNK_SIZE)
    {
        alloc.chunksize = MID_POOL_CHUNK_SIZE;
        alloc.poolstart = meta->mid_pool;
        alloc.poolchunks = meta->mid_poolchunks;
        alloc.bitmapstart = meta->mid_bitmap;
        alloc.bitmapsize = meta->mid_bitmap_size;
    }
    else
    {
        alloc.chunksize = LO_POOL_CHUNK_SIZE;
        alloc.poolstart = meta->lo_pool;
        alloc.poolchunks = meta->lo_poolchunks;
        alloc.bitmapstart = meta->lo_bitmap;
        alloc.bitmapsize = meta->lo_bitmap_size;
    }

    size_t chunksneeded = ceil((double)size / alloc.chunksize);

    // 2:
    bool ok = false;
    while (gallocator_find_start_aligned(alignment, &alloc))
    {
        if (chunksneeded == 1)
        {
            ok = true;
            break;
        }

        // 3:
        size_t chunksfound =
            gallocator_has_n_contiguous_bits(alloc, chunksneeded);

        if (chunksfound == chunksneeded)
        {
            ok = true;
            break;
        }

        /* If those weren't enough completely skip them. */
        allocationctx_advance(chunksfound, &alloc);
    }

    /* MAYBE TODO: We could go back to step 1 and use smaller (or even bigger)
     * chunk-sizes, to try to honor this request. */
    if (!ok)
        return NULL;

    // 4:
    if (gallocator_bitmap_set_bits(alloc, chunksneeded) != 0)
        return NULL;

    // 5:
    void* addr = (void*)(alloc.poolstart + allocationctx_to_offset(&alloc));

    GAllocationMeta* allocmeta =
        (GAllocationMeta*)(addr - sizeof(GAllocationMeta));
    allocmeta->signature = ALLOCATION_SIGNATURE;
    allocmeta->size = size;
    allocmeta->chunksize = alloc.chunksize;
    allocmeta->alignment = alignment;

    return addr;
}

int gallocator_init()
{
    return 0;
}

int gallocator_init_heap(const Heap* heap)
{
    if (!heap)
        return -EINVAL;

    size_t heapsize = heap->pagespan * PAGESIZE;
    size_t metasize =
        bytes_align_up64(sizeof(GAllocatorHeapMeta), HI_POOL_CHUNK_SIZE);

    if (heapsize <= metasize)
        return -ENOMEM;

    /* The heap metadata will be stored at the beggining of the heap. */
    heapsize -= metasize;

    /* This is just a split that kind-of makes sense, but there is room for
     * improvement, especially with larger heaps. */
    size_t lo_pool_size = heapsize / 4;
    size_t mid_pool_size = heapsize / 4;
    size_t hi_pool_size = heapsize / 2;

    size_t lo_poolchunks = lo_pool_size / LO_POOL_CHUNK_SIZE;
    size_t mid_poolchunks = mid_pool_size / MID_POOL_CHUNK_SIZE;
    size_t hi_poolchunks = hi_pool_size / HI_POOL_CHUNK_SIZE;

    size_t lo_bitmap_size = ceil(lo_poolchunks / 8.0);
    size_t mid_bitmap_size = ceil(mid_poolchunks / 8.0);
    size_t hi_bitmap_size = ceil(hi_poolchunks / 8.0);

    heapsize -= lo_bitmap_size + mid_bitmap_size + hi_bitmap_size;

    lo_pool_size = heapsize / 4;
    mid_pool_size = heapsize / 4;
    hi_pool_size = heapsize / 2;

    lo_poolchunks = lo_pool_size / LO_POOL_CHUNK_SIZE;
    mid_poolchunks = mid_pool_size / MID_POOL_CHUNK_SIZE;
    hi_poolchunks = hi_pool_size / HI_POOL_CHUNK_SIZE;

    GAllocatorHeapMeta* meta = (GAllocatorHeapMeta*)heap->vaddr;
    meta->signature = HEAP_SIGNATURE;

    meta->lo_poolchunks = lo_poolchunks;
    meta->mid_poolchunks = mid_poolchunks;
    meta->hi_poolchunks = hi_poolchunks;

    meta->lo_bitmap_size = lo_bitmap_size;
    meta->mid_bitmap_size = mid_bitmap_size;
    meta->hi_bitmap_size = hi_bitmap_size;

    const ptr bitmap_heap_start = heap->vaddr + metasize;
    meta->lo_bitmap = bitmap_heap_start;
    meta->mid_bitmap = bitmap_heap_start + lo_bitmap_size;
    meta->hi_bitmap = bitmap_heap_start + lo_bitmap_size + mid_bitmap_size;

    meta->hi_pool = meta->hi_bitmap + meta->hi_bitmap_size;
    meta->mid_pool = meta->hi_pool + meta->hi_poolchunks * HI_POOL_CHUNK_SIZE;
    meta->lo_pool = meta->mid_pool + meta->mid_poolchunks * MID_POOL_CHUNK_SIZE;

    /** Make sure all pools are aligned on their respective chunk sizes. This
     * helps when allocating aligned addresses. Also it's safe to shrink the
     * pools, and leave the bitmaps the same size, because (I think) we check
     * for both of these bounds everytime we walk a bitmap :)
     */
    {
        ptr tmp_hi_pool = bytes_align_up64(meta->hi_pool, HI_POOL_CHUNK_SIZE);
        ptr tmp_mid_pool =
            bytes_align_up64(meta->mid_pool, MID_POOL_CHUNK_SIZE);
        ptr tmp_lo_pool = bytes_align_up64(meta->lo_pool, LO_POOL_CHUNK_SIZE);

        /* We subtract one cunk at most */
        meta->hi_poolchunks -= (tmp_hi_pool != meta->hi_pool);
        meta->mid_poolchunks -= (tmp_mid_pool != meta->mid_pool);
        meta->lo_poolchunks -= (tmp_lo_pool != meta->lo_pool);

        meta->hi_pool = tmp_hi_pool;
        meta->mid_pool = tmp_mid_pool;
        meta->lo_pool = tmp_lo_pool;
    }

    /* Early-development-trauma sanity check */
    ASSERT(meta->lo_pool % LO_POOL_CHUNK_SIZE == 0);
    ASSERT(meta->mid_pool % MID_POOL_CHUNK_SIZE == 0);
    ASSERT(meta->hi_pool % HI_POOL_CHUNK_SIZE == 0);

    KLOGF(
        DEBUG, "low chunks (%d): %u", LO_POOL_CHUNK_SIZE, meta->lo_poolchunks);
    KLOGF(
        DEBUG,
        "mid chunks (%d): %u",
        MID_POOL_CHUNK_SIZE,
        meta->mid_poolchunks);
    KLOGF(
        DEBUG, "high chunks (%d): %u", HI_POOL_CHUNK_SIZE, meta->hi_poolchunks);

    return 0;
}

void* gallocator_alloc_aligned(size_t size, size_t alignment, const Heap* heap)
{
    GAllocatorHeapMeta* meta = (GAllocatorHeapMeta*)heap->vaddr;
    if (meta->signature != HEAP_SIGNATURE)
        return NULL;

    return gallocator_do_alloc_aligned(size, alignment, meta);
}

ssize_t gallocator_allocation_size(void* addr, const Heap* heap)
{
    GAllocatorHeapMeta* meta = (GAllocatorHeapMeta*)heap->vaddr;
    if (meta->signature != HEAP_SIGNATURE)
        return -EINVAL;

    GAllocationMeta* allocmeta = addr - sizeof(GAllocationMeta);
    if (allocmeta->signature != ALLOCATION_SIGNATURE)
        return -EINVAL;

    return allocmeta->size;
}

ssize_t gallocator_allocation_alignment(void* addr, const Heap* heap)
{
    GAllocatorHeapMeta* meta = (GAllocatorHeapMeta*)heap->vaddr;
    if (meta->signature != HEAP_SIGNATURE)
        return -EINVAL;

    GAllocationMeta* allocmeta = addr - sizeof(GAllocationMeta);
    if (allocmeta->signature != ALLOCATION_SIGNATURE)
        return -EINVAL;

    return allocmeta->alignment;
}

bool gallocator_is_valid_allocation(void* addr, const Heap* heap)
{
    (void)addr;
    (void)heap;
    // FIXME: dang
    return true;
}

void gallocator_free(void* addr, const Heap* heap)
{
    GAllocatorHeapMeta* meta = (GAllocatorHeapMeta*)heap->vaddr;
    if (meta->signature != HEAP_SIGNATURE)
        return;

    GAllocationMeta* allocmeta = addr - sizeof(GAllocationMeta);
    if (allocmeta->signature != ALLOCATION_SIGNATURE)
        return;

    AllocationCtx alloc;
    alloc.chunksize = allocmeta->chunksize;

    switch (allocmeta->chunksize)
    {
    case LO_POOL_CHUNK_SIZE:
        alloc.bitmapstart = meta->lo_bitmap;
        alloc.bitmapsize = meta->lo_bitmap_size;
        alloc.poolstart = meta->lo_pool;
        alloc.poolchunks = meta->lo_poolchunks;
        break;

    case MID_POOL_CHUNK_SIZE:
        alloc.bitmapstart = meta->mid_bitmap;
        alloc.bitmapsize = meta->mid_bitmap_size;
        alloc.poolstart = meta->mid_pool;
        alloc.poolchunks = meta->mid_poolchunks;
        break;

    case HI_POOL_CHUNK_SIZE:
        alloc.bitmapstart = meta->hi_bitmap;
        alloc.bitmapsize = meta->hi_bitmap_size;
        alloc.poolstart = meta->hi_pool;
        alloc.poolchunks = meta->hi_poolchunks;
        break;

    default:
        ASSERT_NOT_HIT();
    }

    size_t chunks = ceil((double)allocmeta->size / allocmeta->chunksize);

    addr_to_allocationctx_offset((ptr)addr, &alloc);

    ASSERT(gallocator_bitmap_clear_bits(alloc, chunks) == 0);
}
