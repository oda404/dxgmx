/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/pagetable.h>

void pagetable_init(PageTable* pt)
{
    memset(pt, 0, sizeof(*pt));
}

void pte_set_frame_base(ptr base, PageTableEntry* pte)
{
    pte->frame_base = base >> 12;
}

ptr pte_frame_base(PageTableEntry* pte)
{
    return pte->frame_base << 12;
}

PageTableEntry* pte_from_vaddr(ptr vaddr, PageTable* pt)
{
    ptr off = vaddr / (PAGESIZE * 512) * 512;
    return &pt->entries[(vaddr / PAGESIZE) - off];
}
