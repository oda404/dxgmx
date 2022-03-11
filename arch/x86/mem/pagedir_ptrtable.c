/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/string.h>
#include <dxgmx/x86/pagedir_ptrtable.h>

void pdpt_init(PageDirectoryPointerTable* pdpt)
{
    memset(pdpt, 0, sizeof(*pdpt));
}

void pdpte_set_pagedir_base(u64 base, PageDirectoryPointerTableEntry* pdpte)
{
    pdpte->pd_base = base >> 12;
}

PageDirectory* pdpte_pagedir_base(const PageDirectoryPointerTableEntry* pdpte)
{
    return (PageDirectory*)(pdpte->pd_base << 12);
}
