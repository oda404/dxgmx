/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_GALLOCATOR_H
#define _DXGMX_MEM_GALLOCATOR_H

#include <dxgmx/mem/heap.h>

int gallocator_init();
int gallocator_init_heap(const Heap* heap);
void* gallocator_alloc_aligned(size_t size, size_t alignment, const Heap* heap);
ssize_t gallocator_allocation_size(void* addr, const Heap* heap);
ssize_t gallocator_allocation_alignment(void* addr, const Heap* heap);
bool gallocator_is_valid_allocation(void* addr, const Heap* heap);
void gallocator_free(void* addr, const Heap* heap);

#endif // !_DXGMX_MEM_GALLOCATOR_H
