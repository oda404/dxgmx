
#include<dxgmx/paging/table.h>
#include<dxgmx/paging/size.h>

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
