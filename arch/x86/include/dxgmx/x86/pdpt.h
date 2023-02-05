/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* Page Directory Pointer Table. Who tf thought this was a good name? */

#ifndef _DXGMX_X86_PDPT_H
#define _DXGMX_X86_PDPT_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/pd.h>

typedef struct _ATTR_PACKED S_PageDirectoryPointerTableEntry
{
    /* The referenced page directory is present in memory. */
    u16 present : 1;
    /* Must be 0. */
    u16 reserved : 2;
    /* Page-level write through. */
    u16 pwt : 1;
    /* Page-level cache disabled. */
    u16 pcd : 1;
    /* Must be 0. */
    u16 reserved2 : 4;
    u16 ignored : 3;
    /* Physical address of the page directory. */
    u64 pd_base : 51;
    /* Must be 0. */
    u16 reserved3 : 1;
} PageDirectoryPointerTableEntry;

typedef struct S_PageDirectoryPointerTable
{
    PageDirectoryPointerTableEntry entries[4];
} PageDirectoryPointerTable;

typedef PageDirectoryPointerTable pdpt_t;
typedef PageDirectoryPointerTableEntry pdpte_t;

/**
 * Initialize a page directory pointer table.
 *
 * 'pdpt' The target pdpt.
 */
void pdpt_init(pdpt_t* pdpt);

/**
 * Set the address of a page directory in the pdpte from a paddr.
 * 'paddr' The virtual address.
 * 'pdpte' The target pdpte.
 */
void pdpte_set_pd_paddr(ptr paddr, pdpte_t*);

/**
 * Set the address of a page directory in the pdpte from a vaddr.
 * 'vaddr' The virtual address.
 * 'pdpte' The target pdpte.
 */
void pdpte_set_pd_vaddr(ptr vaddr, pdpte_t* pdpte);

/**
 * Get a page directory's physical address.
 *
 * 'pdpte' The target pdpte.
 *
 * Returns:
 * The physical address of the page directory.
 */
ptr pdpte_pd_paddr(const pdpte_t* pdpte);

/**
 * Get a page directory's virtual address.
 *
 * 'pdpte' The target pdpte.
 *
 * Returns:
 * The virtual address of the page directory.
 */
pd_t* pdpte_pd_vaddr(const pdpte_t* pdpte);

/**
 * Get the pdpte in which 'vaddr' falls.
 *
 * 'vaddr' The virtual address.
 * 'pdpt' The target pdpt.
 */
pdpte_t* pdpte_from_vaddr(ptr vaddr, pdpt_t* pdpt);

#endif //!_DXGMX_X86_PDPT_H
