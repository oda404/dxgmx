/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_HEAP_H
#define _DXGMX_MEM_HEAP_H

/* pagesize.h is included because we might as well use PAGESIZE, because of
 * Heap::pagespan. */
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/types.h>

typedef struct S_Heap
{
    /* Starting virtual address.  */
    ptr vaddr;
    /* Page span of this heap. */
    size_t pagespan;
} Heap;

bool heap_is_addr_inside(ptr addr, const Heap* heap);

#endif // !_DXGMX_MEM_HEAP_H
