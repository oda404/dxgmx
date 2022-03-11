/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/string.h>
#include <dxgmx/x86/pagedir.h>

void pagedir_init(PageDirectory* pd)
{
    memset(pd, 0, sizeof(*pd));
}

void pde_set_table_base(u64 base, PageDirectoryEntry* pde)
{
    pde->table_base = base >> 12;
}

PageTable* pde_table_base(PageDirectoryEntry* pde)
{
    return (PageTable*)(pde->table_base << 12);
}
