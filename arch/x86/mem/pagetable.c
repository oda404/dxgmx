
#include<dxgmx/x86/pagetable.h>
#include<dxgmx/string.h>

void pagetable_init(PageTable *pt)
{
    memset(pt, 0, sizeof(*pt));
}

void pte_set_frame_base(u64 base, PageTableEntry *pte)
{
    pte->frame_base = base >> 12;
}

u64 pte_frame_base(PageTableEntry *pte)
{
    return pte->frame_base << 12;
}
