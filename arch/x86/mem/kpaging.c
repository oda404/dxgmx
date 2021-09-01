/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/sysidt.h>
#include<dxgmx/cpu.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/mem/kpaging.h>
#include<dxgmx/mem/pagedir.h>
#include<dxgmx/mem/pagetable.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/klog.h>
#include<dxgmx/attrs.h>

static PageDirectory pagedir _ATTR_ALIGNED(PAGE_SIZE);
static PageTable pagetables _ATTR_ALIGNED(PAGE_SIZE);

static void paging_int_handler(
    const InterruptFrame _ATTR_MAYBE_UNUSED *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
    ptr faultaddr = cpu_read_cr2();
    if(faultaddr == 0)
        abandon_ship("NULL dereference in ring 0 :(\n");
    else if(faultaddr < PAGE_SIZE)
        abandon_ship("Tried to access 0-%d page.\n", PAGE_SIZE);
}

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
    pagetable_entry_set_present(0, entry);
    pagetable_entry_set_rw(0, entry);
    
    /* Identity map the first 4MiB */
    for(size_t i = 1; i < 1024; ++i)
    {
        entry = &pagetables.entries[i];
        pagetable_entry_set_frame_base(i * PAGE_SIZE, entry);
        pagetable_entry_set_present(1, entry);
        pagetable_entry_set_rw(1, entry);
    }

    pagedir_entry_set_table_base(&pagetables, &pagedir.entries[0]);
    pagedir_entry_set_present(1, &pagedir.entries[0]);
    pagedir_entry_set_rw(1, &pagedir.entries[0]);

    pagedir_load(&pagedir);

    sysidt_register_callback(ISR14, paging_int_handler);

    paging_enable();

    klog(KLOG_INFO, "Entering paging prison...\n");

    return 0;
}
