/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_PAGING_H
#define _DXGMX_PAGING_H

#include<dxgmx/types.h>
#include<dxgmx/x86/pagetable.h>

void paging_init();
PageTableEntry *paging_pte_from_vaddr(ptr vaddr);

#if defined(_X86_)
void paging_flush_tlb_entries(ptr vaddr);
#endif

#endif //_DXGMX_PAGING_H
