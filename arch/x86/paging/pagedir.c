
#include<dxgmx/paging/paging.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/string.h>
#include<stdbool.h>
#include<dxgmx/kprintf.h>
#include<stddef.h>

void pagedir_init(PageDirectory *dir)
{
    memset((void *)dir, 0, sizeof(PageDirectory));
}

int pagedir_add(
    uint64_t page_base,
    uint64_t frame_base,
    uint16_t flags,
    PageDirectory *dir,
    PageTable tables[1024]
)
{
    if(
        !bw_is_aligned(page_base, PAGE_SIZE) ||
        !bw_is_aligned(frame_base, PAGE_SIZE)
    )
    {
        return 1;
    }

    size_t entry_idx = page_base / PAGE_SIZE;
    size_t table_idx = entry_idx / PAGE_SIZE;
    entry_idx -= table_idx * PAGE_SIZE;

    PageDirectoryEntry *pagedir_entry = &dir->entries[table_idx];
    pagedir_entry->table_base = (uint32_t)&tables[table_idx];

    if(!pagedir_entry->present)
        pagedir_entry->present     = (bool)(flags & PAGEFLAG_PRESENT);

    if(!pagedir_entry->rw)
        pagedir_entry->rw          = (bool)(flags & PAGEFLAG_RW);

    if(!pagedir_entry->user_access)
        pagedir_entry->user_access = (bool)(flags & PAGEFLAG_USER_ACCESS);

    if(!pagedir_entry->write_through)
        pagedir_entry->write_through = (bool)(flags & PAGEFLAG_WRITE_THROUGH);

    if(!pagedir_entry->cache_disabled)
        pagedir_entry->cache_disabled = (bool)(flags & PAGEFLAG_CACHE_DISABLED);

    pagedir_entry->zero = 0;

    PageTable *pagetable = (PageTable *)(uint32_t)pagedir_entry->table_base;
    PageTableEntry *pagetable_entry = &pagetable->entries[entry_idx];
    
    pagetable_entry->frame_base     = frame_base;
    pagetable_entry->present        = (bool)(flags & PAGEFLAG_PRESENT);
    pagetable_entry->rw             = (bool)(flags & PAGEFLAG_RW);
    pagetable_entry->user_access    = (bool)(flags & PAGEFLAG_USER_ACCESS);
    pagetable_entry->write_through  = (bool)(flags & PAGEFLAG_WRITE_THROUGH);
    pagetable_entry->cache_disabled = (bool)(flags & PAGEFLAG_CACHE_DISABLED);
    pagetable_entry->global         = (bool)(flags & PAGEFLAG_GLOBAL);

    //kprintf("%ld ", entry_idx);
    
    return 0;
}
