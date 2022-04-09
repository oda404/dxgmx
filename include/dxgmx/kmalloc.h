/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KMALLOC_H
#define _DXGMX_KMALLOC_H

#include <dxgmx/types.h>

bool kmalloc_init();
void* kmalloc(size_t size);
void* kcalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t alignment);
void kfree(void* addr);
/* This function should only be used for allocations made with
kmalloc(). There is currently no way to reallocate kmalloc_aligned()
allocations other than kfree() and kmalloc_aligned() by yourself. */
void* krealloc(void* addr, size_t size);

#endif //!_DXGMX_KMALLOC_H
