
#include<dxgmx/paging/pgtable.h>
#include<dxgmx/paging/pgsize.h>
#include<dxgmx/stdio.h>

int pgtable_entry_encode(
    uint32_t base,
    uint16_t flags,
    PageTableEntry *entry
)
{
    if(((base + (_PAGE_SIZE - 1)) & ~(_PAGE_SIZE - 1)) != base)
        return 1;

    if((flags & 0b111111111) != flags)
        return 2;
    
    entry->base = base;
    entry->flags = flags;

    return 0;
}
