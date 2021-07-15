
#include<dxgmx/paging/pagetable.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/paging/pagesize.h>
#include<dxgmx/string.h>

void pagetable_init(PageTable *table)
{
    memset((void *)table, 0, sizeof(PageTable));
}

void pagetable_entry_set_present(
    int present,
    PageTableEntry *entry
)
{
    entry->present = present;
}

void pagetable_entry_set_rw(
    int rw,
    PageTableEntry *entry
)
{
    entry->rw = rw;
}

void pagetable_entry_set_user_access(
    int user_acces,
    PageTableEntry *entry
)
{
    entry->user_access = user_acces;
}

void pagetable_entry_set_write_through(
    int write_through,
    PageTableEntry *entry
)
{
    entry->write_through = write_through;
}

void pagetable_entry_set_cache_disabled(
    int cache_disabled,
    PageTableEntry *entry
)
{
    entry->cache_disabled = cache_disabled;
}

void pagetable_entry_set_pat_memtype(
    int pat_memtype,
    PageTableEntry *entry
)
{
    entry->pat_memtype = pat_memtype;
}

void pagetable_entry_set_global(
    int global,
    PageTableEntry *entry
)
{
    entry->global = global;
}

int pagetable_entry_set_frame_base(
    uint32_t frame_base, 
    PageTableEntry *entry
)
{
    if(!bw_is_aligned(frame_base, PAGE_SIZE))
        return 1;

    entry->frame_base = frame_base >> 12;

    return 0;
}

uint32_t pagetable_entry_get_frame_base(PageTableEntry *entry)
{
    return entry->frame_base;
}

int pagetable_entry_get_present(PageTableEntry *entry)
{
    return entry->present;
}

int pagetable_entry_get_rw(PageTableEntry *entry)
{
    return entry->rw;
}

int pagetable_entry_get_user_access(PageTableEntry *entry)
{
    return entry->user_access;
}

int pagetable_entry_get_write_through(PageTableEntry *entry)
{
    return entry->write_through;
}

int pagetable_entry_get_cache_disabled(PageTableEntry *entry)
{
    return entry->cache_disabled;
}

int pagetable_entry_get_pat_memtype(PageTableEntry *entry)
{
    return entry->pat_memtype;
}

int pagetable_entry_get_global(PageTableEntry *entry)
{
    return entry->global;
}
