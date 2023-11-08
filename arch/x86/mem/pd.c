/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/pd.h>

void pd_init(pd_t* pd)
{
    memset(pd, 0, sizeof(*pd));
}

void pde_set_pt_paddr(ptr base, pde_t* pde)
{
    pde->table_base = base >> 12;
}

void pde_set_pt_vaddr(ptr vaddr, pde_t* pde)
{
    pde_set_pt_paddr(mm_kva2pa(vaddr), pde);
}

ptr pde_pt_paddr(pde_t* pde)
{
    return pde->table_base << 12;
}

pt_t* pde_pt_vaddr(pde_t* pde)
{
    ptr ptpaddr = pde_pt_paddr(pde);
    if (!ptpaddr)
        return NULL;

    return (pt_t*)mm_kpa2va(ptpaddr);
}

pde_t* pde_from_vaddr(ptr vaddr, pd_t* pd)
{
    ptr off = vaddr / (PAGESIZE * 512 * 512) * 512;
    return &pd->entries[(vaddr / (PAGESIZE * 512)) - off];
}
