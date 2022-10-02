/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_MM_H
#define _DXGMX_MEM_MM_H

#include <dxgmx/mem/mregmap.h>
#include <dxgmx/mem/paging.h>
#include <dxgmx/types.h>

int mm_init();
/* Get the system memory region map. This map only includes
regions that are available. */
const MemoryRegionMap* mm_get_sys_mregmap();

int mm_init_paging_struct(PagingStruct*);
int mm_destroy_paging_struct(PagingStruct*);
int mm_new_user_page(ptr vaddr, u16 flags, PagingStruct*);
int mm_map_kernel_into_paging_struct(PagingStruct*);
int mm_load_paging_struct(PagingStruct*);
int mm_load_kernel_paging_struct();

/* Translate a kernel virtual address into a physical address. */
ptr mm_kvaddr2paddr(ptr vaddr);

#endif //!_DXGMX_MEM_MM_H
