/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/pagedir.h>
#include<dxgmx/mem/pagetable.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/string.h>
#include<stddef.h>
#include<stdbool.h>

void pagedir_init(PageDirectory *dir)
{
    memset((void *)dir, 0, sizeof(PageDirectory));
}

void pagedir_load(PageDirectory *dir)
{
    asm volatile("movl %0, %%eax; movl %%eax, %%cr3" : : "b"(dir));
}

void pagedir_entry_set_present(
    int present,
    PageDirectoryEntry *entry
)
{
    entry->present = (bool)present;
}

void pagedir_entry_set_rw(
    int rw,
    PageDirectoryEntry *entry
)
{
    entry->rw = (bool)rw;
}

void pagedir_entry_set_user_access(
    int user_acces,
    PageDirectoryEntry *entry
)
{
    entry->user_access = (bool)user_acces;
}

void pagedir_entry_set_cache_disabled(
    int cache_disabled,
    PageDirectoryEntry *entry
)
{
    entry->cache_disabled = (bool)cache_disabled;
}

void pagedir_entry_set_page_size(
    int page_size,
    PageDirectoryEntry *entry
)
{
    entry->page_size = (bool)page_size;
}

int pagedir_entry_set_table_base(
    PageTable *table_base, 
    PageDirectoryEntry *entry
)
{
    if(!bw_is_aligned((uint32_t)table_base, PAGE_SIZE))
        return 1;

    entry->table_base = (uint32_t)table_base >> 12;

    return 0;   
}

int pagedir_entry_get_present(const PageDirectoryEntry *entry)
{
    return entry->present;
}

int pagedir_entry_get_rw(const PageDirectoryEntry *entry)
{
    return entry->rw;
}

int pagedir_entry_get_user_access(const PageDirectoryEntry *entry)
{
    return entry->user_access;
}

int pagedir_entry_get_cache_disabled(const PageDirectoryEntry *entry)
{
    return entry->cache_disabled;
}

int pagedir_entry_get_accessed(const PageDirectoryEntry *entry)
{
    return entry->accessed;
}

int pagedir_entry_get_page_size(const PageDirectoryEntry *entry)
{
    return entry->page_size;
}

PageTable *pagedir_entry_get_table_base(const PageDirectoryEntry *entry)
{
    return (PageTable *)((uint32_t)entry->table_base << 12);
}
