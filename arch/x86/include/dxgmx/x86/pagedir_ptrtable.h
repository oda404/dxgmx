/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_PAGEDIR_PTRTABLE_H
#define _DXGMX_X86_PAGEDIR_PTRTABLE_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/pagedir.h>

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

void pdpt_init(pdpt_t*);
/* This function does not take in a pd_t* as 'base' to avoid confusion with a
 * virtual address, since it excepts a physical address. */
void pdpte_set_pagedir_base(ptr base, pdpte_t*);
/* This function does not return a pd_t*, to avoid "blind" dereferencing since
 * the return value is a physical address, and needs to be translated. */
ptr pdpte_pagedir_base(const pdpte_t*);
pdpte_t* pdpte_from_vaddr(ptr vaddr, pdpt_t* pdpt);

#endif //!_DXGMX_X86_PAGEDIR_PTRTABLE_H
