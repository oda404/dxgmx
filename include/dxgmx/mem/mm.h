/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_MM_H
#define _DXGMX_MEM_MM_H

#include <dxgmx/assert.h>
#include <dxgmx/mem/paging.h>
#include <dxgmx/types.h>

#define FOR_EACH_PAGE(_start, _end, _page)                                     \
    ASSERT(_start % PAGESIZE == 0);                                            \
    for (size_t _page = _start; _page < _end; _page += PAGESIZE)

void mm_init();

int mm_set_page_flags(ptr vaddr, u16 flags, PagingStruct* ps);
int mm_rm_page_flags(ptr vaddr, u16 flags, PagingStruct* ps);

/**
 * Create and map a new page to a given physical adress.
 *
 * No null pointers should be passed to this function.
 *
 * 'vaddr' The starting point of the page.
 * 'paddr' The PAGE_SIZE aligned address of the page frame.
 * 'flags' Flags to apply to the page.
 * 'ps' The target paging struct.
 *
 * Returns:
 * 0 on sucess.
 * -EINVAL on invalid arguments.
 * -ENOMEM on out of memory.
 */
int mm_map_page(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps);
int mm_new_user_page(ptr vaddr, u16 flags, PagingStruct*);

/**
 * Get the kernel's paging structure.
 *
 * Returns:
 * A non-null pointer to the kernel's paging structure.
 */
PagingStruct* mm_get_kernel_paging_struct();

/**
 * Convert a virtual address to a physical adress.
 *
 * No null pointers should be passed to this function.
 *
 * 'vaddr' The virtual adsress.
 * 'ps' The paging struct used for the translation.
 *
 *
 * Returns:
 * The physical adress coresponding to 'vaddr'.
 * NULL if vaddr is not mapped.
 */
ptr mm_va2pa(ptr va, const PagingStruct* ps);

/**
 * Convert a kenel virtual address to a physical address.
 *
 * 'vaddr' The virtual address.
 *
 * Returns:
 * The physical address coresponding to 'vaddr'
 */
ptr mm_kva2pa(ptr kva);

/**
 * Convert a kenel physical address to a virtual address.
 *
 * 'paddr' The physical address.
 *
 * Returns:
 * The virtual address coresponding to 'paddr'
 */
ptr mm_kpa2va(ptr pa);

int mm_init_paging_struct(PagingStruct*);
void mm_destroy_paging_struct(PagingStruct*);
int mm_map_kernel_into_paging_struct(PagingStruct*);
void mm_load_paging_struct(const PagingStruct*);
void mm_load_kernel_paging_struct();

#endif //!_DXGMX_MEM_MM_H
