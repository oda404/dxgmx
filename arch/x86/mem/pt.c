/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/pt.h>

void pt_init(pt_t* pt)
{
    memset(pt, 0, sizeof(*pt));
}

void pte_set_frame_paddr(ptr paddr, pte_t* pte)
{
    pte->frame_base = paddr >> 12;
}

ptr pte_frame_paddr(pte_t* pte)
{
    return pte->frame_base << 12;
}

pte_t* pte_from_vaddr(ptr vaddr, pt_t* pt)
{
    ptr off = vaddr / (PAGESIZE * 512) * 512;
    return &pt->entries[(vaddr / PAGESIZE) - off];
}
