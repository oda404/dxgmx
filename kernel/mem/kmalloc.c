/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/gallocator.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bitwise.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "kmalloc: "
/* KLOG informations about each kmalloc/free call. Very verbose! */
#define KMALLOC_VERBOSE 0

static bool g_kmalloc_up;
/* The kmalloc driver. */
static KMallocDriver g_driver;
/* The kernel heap. */
static Heap g_kheap;

static int kmalloc_check_driver(const KMallocDriver* drv)
{
    /* drv->realloc is optional, because I'm a dipshit. */
    bool valid =
        (drv->name && drv->init && drv->init_heap && drv->alloc_aligned &&
         drv->allocation_size && drv->allocation_alignment &&
         drv->is_valid_allocation && drv->free);

    valid = valid && bw_is_power_of_two(drv->default_alignment);

    if (!valid)
        return -EINVAL;

    return 0;
}

/* kmalloc_aligned() that specifically takes in a heap. */
static void*
kmalloc_aligned_with_heap(size_t size, size_t alignment, Heap* heap)
{
    void* addr = g_driver.alloc_aligned(size, alignment, heap);

#if KMALLOC_VERBOSE == 1
    if (addr)
    {
        KLOGF(
            DEBUG,
            "Allocated 0x%X bytes at 0x%p (alignment 0x%X)",
            size,
            addr,
            alignment);
    }
    else
    {
        KLOGF(
            DEBUG,
            "Failed to allocate 0x%X bytes (alignment 0x%X)",
            size,
            alignment);
    }
#endif

    return addr;
}

/* kfree() that specifically takes in a heap. */
static void kfree_with_heap(void* addr, Heap* heap)
{
    g_driver.free(addr, heap);

#if KMALLOC_VERBOSE == 1
    KLOGF(DEBUG, "Freed address 0x%p", addr);
#endif
}

int kmalloc_register_kernel_heap(Heap heap)
{
    /* We can only register one kernel heap. NOTE: should we really have this
     * restriction ? */
    if (g_kheap.vaddr || g_kheap.pagespan)
        return -EEXIST;

    /* The kernel heap should start after the kernel image. */
    if (heap.vaddr < kimg_vaddr() + kimg_size() || heap.pagespan == 0)
        return -EINVAL;

    /* FIXME: this is an ugly hack to make sure the kernel heap is zeroed out.
    Right now we map the first 2 MiB of memory to 3 GiB. The kernel is loaded at
    1 MiB, and is around ~300 KiB. Everything after the kernel image is
    considered kernel heap, part of which is already mapped. That mapped portion
    may have garbage data, and we zero it out here. Note that when an
    unmapped part of the kernel heap gets accessed, and as a result mapped, it's
    automatically zeroed out. */
    size_t sz = bytes_align_up64(heap.vaddr, 0x100'000) - heap.vaddr;
    memset((void*)heap.vaddr, 0, sz);

    g_kheap = heap;

    return 0;
}

_INIT int kmalloc_init()
{
    /* For now the gallocator is the only one we have... Optimistically we would
     * build allocators as modules, but right now we only have a single 'level'
     * of modules that gets initialized long after kmalloc came online. */
    g_driver = (KMallocDriver){
        .name = "gallocator",
        .priority = 1,
        .default_alignment = sizeof(ptr),
        .init = gallocator_init,
        .init_heap = gallocator_init_heap,
        .alloc_aligned = gallocator_alloc_aligned,
        .realloc = NULL, /* Let krealloc do it the 'dumb' way. */
        .allocation_size = gallocator_allocation_size,
        .allocation_alignment = gallocator_allocation_alignment,
        .is_valid_allocation = gallocator_is_valid_allocation,
        .free = gallocator_free};

    int st = kmalloc_check_driver(&g_driver);
    if (st < 0)
        return st;

    /* Init the driver. */
    st = g_driver.init();
    if (st < 0)
        return st;

    /*  Init the kernel heap. */
    st = g_driver.init_heap(&g_kheap);
    if (st < 0)
        return st;

    g_kmalloc_up = true;

    return 0;
}

void* kmalloc(size_t size)
{
    return kmalloc_aligned(size, g_driver.default_alignment);
}

void* kcalloc(size_t size)
{
    void* addr = kmalloc(size);
    if (addr)
        memset(addr, 0, size);

    return addr;
}

/* Small note: all kmalloc functions basically boil down to this one. */
void* kmalloc_aligned(size_t size, size_t alignment)
{
    if (UNLIKELY(!g_kmalloc_up))
        return NULL;

    if (!size)
        return NULL;

    if (!bw_is_power_of_two(alignment))
        panic("kmalloc_aligned: alignment is not a power of two!");

    Heap* heap = &g_kheap;

    void* addr = kmalloc_aligned_with_heap(size, alignment, heap);

    /* Sanity check */
    ASSERT(((ptr)addr % alignment) == 0);

    return addr;
}

void kfree(void* addr)
{
    if (UNLIKELY(!g_kmalloc_up))
        return;

    if (!addr)
        panic("kfree: Tried to kfree a NULL address!");

    Heap* heap = &g_kheap;
    if (!heap_is_addr_inside((ptr)addr, heap))
        panic("kfree: Tried to free an invalid address (0x%p)!", addr);

    kfree_with_heap(addr, heap);
}

void* krealloc(void* addr, size_t size)
{
    if (UNLIKELY(!g_kmalloc_up))
        return NULL;

    if (!size)
        return NULL;

    Heap* heap = &g_kheap;
    if (addr && !heap_is_addr_inside((ptr)addr, heap))
        return NULL;

    if (addr)
    {
        /* If the driver implements 'realloc', we let it do it's thing. */
        if (g_driver.realloc)
            return g_driver.realloc(addr, size, heap);

        /* If not, we do it outselvers, but of course, since we don't know how
         * the driver works, it's gonna be a less-than-optimal solution. */
        size_t prevsize;
        size_t alignment;

        /* Check and set size. */
        ssize_t res = g_driver.allocation_size(addr, heap);
        if (res <= 0)
        {
            KLOGF(ERR, "%s: allocation_size() returned %d", g_driver.name, res);
            return NULL;
        }
        prevsize = res;

        /* Simple enough */
        if (prevsize == size)
            return addr;

        /* Check and set alignment. */
        res = g_driver.allocation_alignment(addr, heap);
        if (res < 0)
        {
            KLOGF(
                ERR,
                "%s: allocation_alignment() returned %d",
                g_driver.name,
                res);
            return NULL;
        }
        alignment = res;
        if (!bw_is_power_of_two(alignment))
        {
            panic(
                "krealloc: %s: returned non-power of two alignment!",
                g_driver.name);
        }

        /* Allocate a new chunk of memory, copy the old memory into the new one,
         * and free the old one. We don't free the old allocation first, to make
         * sure that if the new allocation fails, the old one is still valid, as
         * per spec. */
        void* ret = kmalloc_aligned_with_heap(size, alignment, heap);
        if (!ret)
            return NULL;

        memcpy(ret, addr, (prevsize < size) ? prevsize : size);
        kfree_with_heap(addr, heap);
        return ret;
    }

    /* addr is null, so we treat this lile a simple kmalloc() call. */
    return kmalloc_aligned_with_heap(size, g_driver.default_alignment, heap);
}
