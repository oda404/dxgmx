/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/kpaging.h>
#include<dxgmx/mem/pagedir.h>
#include<dxgmx/mem/pagetable.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/klog.h>
#include<dxgmx/attrs.h>
#include<stddef.h>
#include<stdbool.h>

static PageDirectory pagedir _ATTR_ALIGNED(PAGE_SIZE);
static PageTable pagetables _ATTR_ALIGNED(PAGE_SIZE);

PageDirectory *kpagedir_get()
{
    return &pagedir;
}

int kpaging_init()
{
    pagedir_init(&pagedir);
    pagetable_init(&pagetables);
    
    PageTableEntry *entry = &pagetables.entries[0];
    pagetable_entry_set_frame_base(0, entry);
    pagetable_entry_set_present(false, entry);
    pagetable_entry_set_rw(false, entry);
    
    /* Identity map the first 4MiB */
    for(size_t i = 1; i < 1024; ++i)
    {
        entry = &pagetables.entries[i];
        pagetable_entry_set_frame_base(i * PAGE_SIZE, entry);
        pagetable_entry_set_present(true, entry);
        pagetable_entry_set_rw(true, entry);
    }

    pagedir_entry_set_table_base(&pagetables, &pagedir.entries[0]);
    pagedir_entry_set_present(true, &pagedir.entries[0]);
    pagedir_entry_set_rw(true, &pagedir.entries[0]);

    pagedir_load(&pagedir);
    paging_enable();

    klog(KLOG_INFO, "Entering paging prison...\n");

    return 0;
}
