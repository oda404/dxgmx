/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_PAGING_PAGETABLE_H
#define _DXGMX_X86_PAGING_PAGETABLE_H

#include<stdint.h>
#include<dxgmx/compiler_attrs.h>

/*
 * Structure of a page table entry
 * 
 * |31                                    12|11   9|8                        0|
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
 * | physical page base 4 KiB/MiB aligned   |unused|G |0 |D |A |C |W |U |R |P |
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
*/



typedef struct
_ATTR_PACKED S_PageTableEntry
{
    /* 1 if the page is present in memory. */
    uint8_t  present:        1;
    /* 1 if the page is read/write, 0 if it's read only. */
    uint8_t  rw:             1;
    /* 1 if the page can be accessed by both kernel and users, 0 if the page can only be accessed by the kernel. */
    uint8_t  user_access:    1;
    /* 1 if the page has write through enabled. */
    uint8_t  write_through:  1;
    /* 1 if the page has caching disabled. */
    uint8_t  cache_disabled: 1;
    /* Set to 1 by the CPU if the page has been read/written to. Must be manually cleared if needed. */
    uint8_t  accessed:       1;
    /* Set to 1 by the CPU if the page has been written to. Must be manually cleared if needed. */
    uint8_t  dirty:          1;
    /* If PAT is supported it indicates the memory type. Otherwise must be 0. */
    uint8_t  pat_memtype:    1;
    /* 1 if the page should be kept in the TLB between page dir flushes. */
    uint8_t  global:         1;
    /* Free for use. */
    uint8_t  unused:         3;
    /* 4KiB or 4MiB aligned address of the page frame. */
    uint32_t frame_base:     20;
} PageTableEntry;

typedef struct S_PageTable
{
    PageTableEntry entries[1024];
} PageTable;

void pagetable_init(PageTable *table);

void pagetable_entry_set_present(int present, PageTableEntry *entry);
void pagetable_entry_set_rw(int rw, PageTableEntry *entry);
void pagetable_entry_set_user_access(int user_acces,PageTableEntry *entry);
void pagetable_entry_set_write_through(int write_through, PageTableEntry *entry);
void pagetable_entry_set_cache_disabled(int cache_disabled, PageTableEntry *entry);
void pagetable_entry_set_pat_memtype(int pat_memtype, PageTableEntry *entry);
void pagetable_entry_set_global(int global, PageTableEntry *entry);
int  pagetable_entry_set_frame_base(uint32_t frame_base,  PageTableEntry *entry);

int pagetable_entry_get_present(PageTableEntry *entry);
int pagetable_entry_get_rw(PageTableEntry *entry);
int pagetable_entry_get_user_access(PageTableEntry *entry);
int pagetable_entry_get_write_through(PageTableEntry *entry);
int pagetable_entry_get_cache_disabled(PageTableEntry *entry);
int pagetable_entry_get_pat_memtype(PageTableEntry *entry);
int pagetable_entry_get_global(PageTableEntry *entry);
uint32_t pagetable_entry_get_frame_base(PageTableEntry *entry);

#endif //_DXGMX_X86_PAGING_PAGETABLE_H
