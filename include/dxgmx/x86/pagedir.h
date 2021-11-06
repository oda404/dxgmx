/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_PAGING_PAGEDIR_H
#define _DXGMX_X86_PAGING_PAGEDIR_H

#include<stdint.h>
#include<dxgmx/compiler_attrs.h>
#include<dxgmx/x86/pagetable.h>

/*
 * Structure of a page directory entry
 * 
 * |31                                    12|11   9|8                        0|
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
 * | page table base                        |unused|G |S |0 |A |D |W |U |R |P |
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
*/

typedef struct 
_ATTR_PACKED S_PageDirectoryEntry
{
    /* 1 if the page table is present in memory. */
    uint8_t  present:        1;
    /* 1 if the page table is read/write, 0 if it's read only. */
    uint8_t  rw:             1;
    /* 1 if the page table can be accessed by both kernel and users, 0 if the can only be accessed by the kernel. */
    uint8_t  user_access:    1;
    /* 1 if the page table has write through enabled. */
    uint8_t  write_through:  1;
    /* 1 if the page table has caching disabled. */
    uint8_t  cache_disabled: 1;
    /* Set to 1 by the CPU if the page table has been accessed. Must be manually cleared if needed. */
    uint8_t  accessed:       1;
    /* Must be 0. */
    uint8_t  zero:           1;
    /* 1 if the pages in this directory are 4MiB, 0 if they are 4KiB. */
    uint8_t  page_size:      1;
    /* Ignored. */
    uint8_t  ignored:        1;
    /* Free for use. */
    uint8_t  unused:         3;
    /* 4KiB aligned base address of the page table. */
    uint32_t table_base:     20;
} PageDirectoryEntry;

typedef struct S_PageDirectory
{
    PageDirectoryEntry entries[1024];
} PageDirectory;

/**
 *  Zeroes out the page directory. 
*/
void pagedir_init(PageDirectory *dir);
/** 
 * Loads the address of the page directory into the cr3 register. 
*/
void pagedir_load(PageDirectory *dir);

void pagedir_entry_set_present(int present, PageDirectoryEntry *entry);
void pagedir_entry_set_rw(int rw, PageDirectoryEntry *entry);
void pagedir_entry_set_user_access(int user_access, PageDirectoryEntry *entry);
void pagedir_entry_set_write_through(int write_through, PageDirectoryEntry *entry);
void pagedir_entry_set_cache_disabled(int cache_disabled, PageDirectoryEntry *entry);
void pagedir_entry_set_page_size(int page_size, PageDirectoryEntry *entry);
int pagedir_entry_set_table_base(PageTable *table_base, PageDirectoryEntry *entry);

int pagedir_entry_get_present(const PageDirectoryEntry *entry);
int pagedir_entry_get_rw(const PageDirectoryEntry *entry);
int pagedir_entry_get_user_access(const PageDirectoryEntry *entry);
int pagedir_entry_get_write_through(const PageDirectoryEntry *entry);
int pagedir_entry_get_cache_disabled(const PageDirectoryEntry *entry);
int pagedir_entry_get_accessed(const PageDirectoryEntry *entry);
int pagedir_entry_get_page_size(const PageDirectoryEntry *entry);
PageTable *pagedir_entry_get_table_base(const PageDirectoryEntry *entry);

#endif // _DXGMX_X86_PAGING_PAGEDIR_H
