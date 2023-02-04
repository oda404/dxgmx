/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_MM_H
#define _DXGMX_MEM_MM_H

#include <dxgmx/mem/mregmap.h>
#include <dxgmx/mem/paging.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bitwise.h>

/* Execute permissions */
#define PAGE_X BW_BIT(0)
/* Write permissions */
#define PAGE_W BW_BIT(1)
/* Read permissions */
#define PAGE_R BW_BIT(2)
/* R/W/X permissions mask.  */
#define PAGE_ACCESS_MODE (PAGE_X | PAGE_W | PAGE_R)
/* Page can be accesed by userspace */
#define PAGE_USER BW_BIT(3)

int mm_init();
/* Get the system memory region map. This map only includes
regions that are available. */
const MemoryRegionMap* mm_get_sys_mregmap();

int mm_init_paging_struct(PagingStruct*);
int mm_destroy_paging_struct(PagingStruct*);
int mm_map_kernel_into_paging_struct(PagingStruct*);
int mm_load_paging_struct(PagingStruct*);
int mm_load_kernel_paging_struct();

/**
 * Set a new set flags for a page.
 *
 * No null pointers should be passed to this function.
 *
 * 'vaddr' The starting point of the page.
 * 'flags' New flags. (See PAGE_*)
 * 'ps' The target paging struct.
 *
 * Returns:
 * 0 on sucess.
 * -EINVAL in case the page isn't mapped.
 */
int mm_set_page_flags(ptr vaddr, u16 flags, PagingStruct* ps);

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
int mm_new_page(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps);

/**
 * Create and map a new page to a an arbitrary page frame. The difference
 * between this function and mm_new_page is that the page frame is arbitrarily
 * allocated by falloc_one_user and flags will have PAGE_USER set.
 *
 * No null pointers should be passed to this function.
 *
 * 'vaddr' The starting point of the page.
 * 'flags' Flags to apply to the page.
 * 'ps' The target paging struct.
 *
 * Returns:
 * 0 on sucess.
 * -EINVAL on invalid arguments.
 * -ENOMEM on out of memory.
 */
int mm_new_user_page(ptr vaddr, u16 flags, PagingStruct*);

/* Translate a kernel virtual address into a physical address. */
ptr mm_kvaddr2paddr(ptr vaddr);

#endif //!_DXGMX_MEM_MM_H
