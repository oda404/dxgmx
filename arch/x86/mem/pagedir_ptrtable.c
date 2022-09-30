/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/string.h>
#include <dxgmx/x86/pagedir_ptrtable.h>

void pdpt_init(pdpt_t* pdpt)
{
    memset(pdpt, 0, sizeof(*pdpt));
}

void pdpte_set_pagedir_base(ptr base, pdpte_t* pdpte)
{
    pdpte->pd_base = base >> 12;
}

ptr pdpte_pagedir_base(const pdpte_t* pdpte)
{
    return pdpte->pd_base << 12;
}

pdpte_t* pdpte_from_vaddr(ptr vaddr, pdpt_t* pdpt)
{
    return &pdpt->entries[vaddr / GIB];
}
