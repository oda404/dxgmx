/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KMALLOC_H
#define _DXGMX_KMALLOC_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/mem/heap.h>
#include <dxgmx/types.h>

typedef struct S_KMallocStatistics
{
    /* Nuumber of allocations made since kmalloc came online. */
    size_t total_allocations;
    /* Number of frees since kmalloc came online. */
    size_t total_frees;
    /* How much memory kmalloc has allocated since it came online. */
    size_t total_allocated;
    /* How much memory kmalloc has freed since it came online. */
    size_t total_freed;

} KMallocStatistics;

/**
 * Kmalloc implementation details.
 * All allocations made by a driver shall be inside the heap they are passed.
 * Note how all function pointers that have something to do with memory
 * allocation/freeing/info take in a 'heap'. When an 'addr' is given alongside a
 * 'heap', 'addr' is guaranteed to be inside the memory range represented by
 * said 'heap'. All 'heap' objects and 'addr' pointers passed to any functions
 * are guaranteed to be non-null.
 */
typedef struct S_KMallocDriver
{
    /* Name of this driver */
    char* name;

    /* Priority of this driver over other drivers. */
    size_t priority;

    /* Initialize this driver. */
    int (*init)();

    /* Initialize 'heap' to be used with this driver. */
    int (*init_heap)(const Heap* heap);

    /** Allocate 'size' bytes aligned on 'alignment' bytes.
     * 'size' is guaranteed to be > 0.
     * 'alignment' is guaranteed to be a power of two.
     */
    void* (*alloc_aligned)(size_t size, size_t alignment, const Heap* heap);

    /**
     * Reallocate a previous allocation, to be of (at least) 'newsize' bytes.
     *
     * 'addr' needs to mentain it's original alignment (the one that was used in
     * the alloc_aligned() call).
     *
     * If the reallocation fails, the original one needs to remain valid, and
     * the function shall return NULL.
     *
     * If the reallocation changes it's address, the new address will be
     * returned, and the memory copied accordingly from the original allocation.
     *
     * 'newsize' is guaranteed to be > 0.
     *
     * Returns an address on success, NULL on failure.
     */
    void* (*realloc)(void* addr, size_t newsize, const Heap* heap);

    /** Get the size of an allocation.
     * Returns negative on error.
     */
    ssize_t (*allocation_size)(void* addr, const Heap* heap);

    /** Get the alignment of an allocation.
     * Returns negative on error.
     */
    ssize_t (*allocation_alignment)(void* addr, const Heap* heap);

    /**
     * Returns true if 'addr' is a valid allocated chunk of memory, 'owned' by
     * this driver.
     */
    bool (*is_valid_allocation)(void* addr, const Heap* heap);

    /** Free an allocated address.
     */
    void (*free)(void* addr, const Heap* heap);

    /* Default alignment that should be used when allocating memeory with a
     * function that doesn't explicitly take in the alignnent, ex: kmalloc().
     * Must be a power of two.
     */
    size_t default_alignment;
} KMallocDriver;

/**
 * Initialize kmalloc.
 * Returns 0 on success, negative on error.
 */
int kmalloc_init();

/* Registering a heap to kmalloc implies giving kmalloc full ownersip of the
 * memory range represented by said heap. No memory in that range is to be used
 * outside kmalloc/without the use of pointers returned by kmalloc, as it may be
 * overwritten/wrongly-interpreted by kmalloc. There is one restriction imposed
 * on a 'kernel heap': it's starting address needs to be
 * >= the end of the kernel image or inside the kernel image. This is done for
 * multiple reasons:
 *
 *  1: The kernel will make sure that it's heap is higher half mapped in the
 * same fashion that the kernel image is. This
 * basically means that the virtual mapping of everything kernel-related,
 * starting from the 3GiB going up to however big the kernel
 * heap is, is just a higher half mapping from 1MiB to however big the kernel
 * heap is.
 *
 * 2: If the heap is higher-half mapped, then n contiguous pages in virtual
 * space are also contiguous in physical space.
 *
 * 3: Translations from physical to virtual addresses become trivial (just add
 * the _kernel_map_offset to the physical address).
 *
 * 4: This also helps mentally separate everything kernel related,
 * as the kernel itself is mapped at a high address, with nothing coming
 * after it (except it's heap, of course). This can be seen easily, just by
 * looking at an address and seeing if it's bigger than _kernel_map_offset.
 */
int kmalloc_register_heap(Heap heap);

int kmalloc_use_heap(size_t id);

bool kmalloc_owns_va(ptr va);

/* Kernel version of malloc. */
void* kmalloc(size_t size);

/* Kernel version of calloc. Except this only takes in the size, instead of the
 * element size and number of elements. */
void* kcalloc(size_t size);

/* kmalloc but the returned address is guaranteed to be aligned on 'alignment'
 * bytes. */
void* kmalloc_aligned(size_t size, size_t alignment);

/* Kernel version of realloc. If 'addr' was returned by a call to
 * kmalloc_aligned(), then new address (if changed) is guaranteed to keep it's
 * alignemnt. */
void* krealloc(void* addr, size_t size);

/* Kernel version of free. */
void kfree(void* addr);

void kmalloc_dump_statistics();

#endif //!_DXGMX_KMALLOC_H
