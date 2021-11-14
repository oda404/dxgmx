
#ifndef _DXGMX_X86_PAGEDIR_PTRTABLE_H
#define _DXGMX_X86_PAGEDIR_PTRTABLE_H

#include<dxgmx/compiler_attrs.h>
#include<dxgmx/types.h>

typedef struct
_ATTR_PACKED S_PageDirectoryPointerTableEntry
{
    /* The referenced page directory is present in memory. */
    u8 present: 1;
    /* Must be 0. */
    u8 reserved: 2;
    /* Page-level write through. */
    u8 pwt: 1;
    /* Page-level cache disabled. */
    u8 pcd: 1;
    /* Must be 0. */
    u8 reserved2: 4;
    u8 ignored: 3;
    /* Physical address of the page directory. */
    u64 pd_base: 51;
    /* Must be 0. */
    u8 reserved3: 1;
} PageDirectoryPointerTableEntry;

typedef struct S_PageDirectoryPointerTable
{
    PageDirectoryPointerTableEntry entries[4];
} PageDirectoryPointerTable;

void pdpt_init(PageDirectoryPointerTable *);
void pdpte_set_pagedir_base(u64 base, PageDirectoryPointerTableEntry *);
u64 pdpte_pagedir_base(PageDirectoryPointerTableEntry *);

#endif //!_DXGMX_X86_PAGEDIR_PTRTABLE_H
