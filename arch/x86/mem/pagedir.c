/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/pagedir.h>

void pagedir_init(pd_t* pd)
{
    memset(pd, 0, sizeof(*pd));
}

void pde_set_table_base(ptr base, pde_t* pde)
{
    pde->table_base = base >> 12;
}

ptr pde_table_base(pde_t* pde)
{
    return pde->table_base << 12;
}

pde_t* pde_from_vaddr(ptr vaddr, pd_t* pd)
{
    ptr off = vaddr / (PAGESIZE * 512 * 512) * 512;
    return &pd->entries[(vaddr / (PAGESIZE * 512)) - off];
}
