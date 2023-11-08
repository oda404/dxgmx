/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_PAGING_H
#define _DXGMX_MEM_PAGING_H

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bitwise.h>

/* Execute permissions */
#define PAGE_X BW_BIT(0)
/* Write permissions */
#define PAGE_W BW_BIT(1)
/* Read permissions */
#define PAGE_R BW_BIT(2)
/* Page can be accesed by userspace */
#define PAGE_USER BW_BIT(3)
/* Page is mapped to physical memory */
#define PAGE_PRESENT BW_BIT(4)

#define PAGE_RW (PAGE_R | PAGE_W)
#define PAGE_RX (PAGE_R | PAGE_X)
#define PAGE_WX (PAGE_W | PAGE_X)
#define PAGE_ACCESS_MODE (PAGE_X | PAGE_W | PAGE_R)

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
