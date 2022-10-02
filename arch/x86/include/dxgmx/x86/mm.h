/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_MM_H
#define _DXGMX_X86_MM_H

#include <dxgmx/x86/pagedir.h>
#include <dxgmx/x86/pagedir_ptrtable.h>
#include <dxgmx/x86/pagetable.h>

/* X86 specific API for the mm. */

int mm_reserve_acpi_range(ptr base, size_t size);

/* Flush the whole TLB */
void mm_tlb_flush_whole();

/* Flush a single address from the TLB. */
void mm_tlb_flush_single(ptr vaddr);

/* Gets the PageDirectory in which 'vaddr' falls. */
pd_t* pd_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt);

/* Gets the PageDirectoryEntry in which 'vaddr' falls. */
pde_t* pde_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt);

pt_t* pt_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt);

pte_t* pte_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt);

pdpt_t* mm_kernel_pdpt();

/* The physical address of a virtual address for a given 'pdpt' */
ptr mm_vaddr2paddr(ptr vaddr, const pdpt_t* pdpt);

#endif // !_DXGMX_X86_MM_H
