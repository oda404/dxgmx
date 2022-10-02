/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_PAGEDIR_H
#define _DXGMX_X86_PAGEDIR_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/pagetable.h>

typedef struct _ATTR_PACKED S_PageDirectoryEntry
{
    /* 1 if the page table is present in memory. */
    u8 present : 1;
    /* 1 if the page table is read/write, 0 if it's read only. */
    u8 writable : 1;
    /* 1 if the page table can be accessed by both kernel and users, 0 if the
     * can only be accessed by the kernel. */
    u8 user_access : 1;
    /* 1 if the page table has write through enabled. */
    u8 writehrough : 1;
    /* 1 if the page table has caching disabled. */
    u8 cache_disabled : 1;
    /* Set to 1 by the CPU if the page table has been accessed. Must be manually
     * cleared if needed. */
    u8 accessed : 1;
    /* Must be 0. */
    u8 zero : 1;
    /* 1 if the pages in this directory are 4MiB, 0 if they are 4KiB. */
    u8 page_size : 1;
    /* Ignored. */
    u8 ignored : 1;
    /* Free for use. */
    u8 unused : 3;
    /* 4KiB aligned base address of the page table. */
    u64 table_base : 50;
    /* Should be 0. */
    u8 reserved : 1;
    /* Page table can't be executed from. */
    u8 exec_disable : 1;
} PageDirectoryEntry;

typedef struct S_PageDirectory
{
    PageDirectoryEntry entries[512];
} PageDirectory;

typedef PageDirectory pd_t;
typedef PageDirectoryEntry pde_t;

void pagedir_init(PageDirectory* pd);
/* This function does not take in a pt_t* as 'base' to avoid confusion with a
 * virtual address, since it excepts a physical address. */
void pde_set_table_base(ptr base, PageDirectoryEntry* pde);
/* This function does not return a pt_t*, to avoid "blind" dereferencing since
 * the return value is a physical address, and needs to be translated. */
ptr pde_table_base(PageDirectoryEntry* pde);
PageDirectoryEntry* pde_from_vaddr(ptr vaddr, PageDirectory* pd);

#endif //!_DXGMX_X86_PAGEDIR_H
