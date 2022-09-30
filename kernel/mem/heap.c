/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/mem/heap.h>

bool heap_is_addr_inside(ptr addr, const Heap* heap)
{
    size_t heapsize = heap->pagespan * PAGESIZE;
    return (addr >= heap->vaddr) && (addr < heap->vaddr + heapsize);
}
