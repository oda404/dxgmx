
#include<dxgmx/paging/pagedir.h>
#include<dxgmx/paging/pgtable.h>
#include<dxgmx/paging/pgsize.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/stdio.h>
#include<dxgmx/string.h>
#include<stddef.h>

void pagedir_init(PageDirectory *dir)
{
    __memset((void *)dir, 0, sizeof(PageDirectory));
}

int pagedir_map_addr(
    uint64_t page_base,
    uint64_t frame_base,
    uint16_t flags,
    PageDirectory *dir
)
{
    if(
        !bw_is_aligned(page_base, _PAGE_SIZE) ||
        !bw_is_aligned(frame_base, _PAGE_SIZE)
    )
    {
        return 1;
    }

    size_t entry_idx = page_base / _PAGE_SIZE;
    size_t table_idx = entry_idx / _PAGE_SIZE;
    entry_idx -= table_idx * _PAGE_SIZE;

    kprintf("%d %d\n", (uint32_t)entry_idx, (uint32_t)table_idx);

    PageTable *pgtable = (PageTable*)dir->entries[table_idx].table_base;

    pgtable->entries[entry_idx].base = frame_base;
    return 0;
}
