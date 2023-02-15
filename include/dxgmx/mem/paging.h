/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_PAGING_H
#define _DXGMX_MEM_PAGING_H

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/types.h>

typedef struct S_Page
{
    ptr vaddr;
} Page;

typedef struct S_PagingStruct
{
    Page* allocated_pages;
    size_t allocated_pages_size;
    size_t allocated_pages_capacity;

    /* Whatever architecture specific struct is being used. */
    void* data;
} PagingStruct;

/**
 * Initialize a paging struct.
 *
 * 'ps' Non NULL paging struct.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memmory.
 */
int pagingstruct_init(PagingStruct* ps);

/**
 * Destroy a paging struct. Note that PagingStruct::data is not freed, that is
 * the caller's job.
 *
 * 'ps' Non NULL paging struct.
 *
 * Returns:
 * 0 on success.
 */
int pagingstruct_destroy(PagingStruct* ps);

/**
 * Track an allocated page. Everytime we allocate a page we also track it's
 * address here. We do this so we don't have to walk the whole paging structure
 * when freeing them later on.
 *
 * 'page' The page to track. The page will be copied (local pointers are ok).
 * 'ps' Non NULL paging struct.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory.
 *
 */
int pagingstruct_track_page(const Page* page, PagingStruct* ps);

#endif // !_DXGMX_MEM_PAGING_H
