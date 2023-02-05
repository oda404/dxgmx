/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/mem/mm.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/pdpt.h>

void pdpt_init(pdpt_t* pdpt)
{
    memset(pdpt, 0, sizeof(pdpt_t));
}

void pdpte_set_pd_paddr(ptr paddr, pdpte_t* pdpte)
{
    pdpte->pd_base = paddr >> 12;
}

void pdpte_set_pd_vaddr(ptr vaddr, pdpte_t* pdpte)
{
    pdpte_set_pd_paddr(mm_kvaddr2paddr(vaddr), pdpte);
}

ptr pdpte_pd_paddr(const pdpte_t* pdpte)
{
    return pdpte->pd_base << 12;
}

pd_t* pdpte_pd_vaddr(const pdpte_t* pdpte)
{
    ptr pdpaddr = pdpte_pd_paddr(pdpte);
    if (!pdpaddr)
        return NULL;

    return (pd_t*)mm_kpaddr2vaddr(pdpaddr);
}

pdpte_t* pdpte_from_vaddr(ptr vaddr, pdpt_t* pdpt)
{
    return &pdpt->entries[vaddr / GIB];
}
