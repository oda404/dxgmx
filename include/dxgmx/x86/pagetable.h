
#ifndef _DXGMX_X86_PAGETABLE_H
#define _DXGMX_X86_PAGETABLE_H

#include<dxgmx/compiler_attrs.h>
#include<dxgmx/types.h>

typedef struct
_ATTR_PACKED S_PageTableEntry
{
    /* 1 if the page is present in memory. */
    u8  present:        1;
    /* 1 if the page is read/write, 0 if it's read only. */
    u8  writable:             1;
    /* 1 if the page can be accessed by both kernel and users, 0 if the page can only be accessed by the kernel. */
    u8  user_access:    1;
    /* 1 if the page has write through enabled. */
    u8  writehrough:  1;
    /* 1 if the page has caching disabled. */
    u8  cache_disabled: 1;
    /* Set to 1 by the CPU if the page has been read/written to. Must be manually cleared if needed. */
    u8  accessed:       1;
    /* Set to 1 by the CPU if the page has been written to. Must be manually cleared if needed. */
    u8  dirty:          1;
    /* If PAT is supported it indicates the memory type. Otherwise must be 0. */
    u8  pat_memtype:    1;
    /* 1 if the page should be kept in the TLB between page dir flushes. */
    u8  global:         1;
    /* Free for use. */
    u8  unused:         3;
    /* 4KiB or 4MiB aligned address of the page frame. */
    u64 frame_base:     50;
    /* Should be 0 */
    u8 reserved:        1;
    /* Page can't be executed from. */
    u8 exec_disable:    1;
} PageTableEntry;

typedef struct S_PageTable
{
    PageTableEntry entries[512];
} PageTable;

void pagetable_init(PageTable *pt);
void pte_set_frame_base(u64 base, PageTableEntry *pte);
u64 pte_frame_base(PageTableEntry *pte);

#endif //!_DXGMX_X86_PAGETABLE_H
