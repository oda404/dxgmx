/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/paging.h>
#include<dxgmx/x86/pagedir.h>
#include<dxgmx/x86/pagetable.h>
#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/compiler_attrs.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/klog.h>
#include<dxgmx/cpu.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/kinfo.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/math.h>
#include<dxgmx/todo.h>

static PageDirectory g_pagedir _ATTR_ALIGNED(PAGE_SIZE);
static PageTable g_pagetables[1];

static void paging_isr(
    const InterruptFrame *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))

    ptr faultaddr = cpu_read_cr2();

    if(PAGEFAULT_IS_PROT_VIOL(frame->code))
    {
        abandon_ship(
            "Page protection violation: tried to %s 0x%08lX. Not proceeding.\n", 
            PAGEFAULT_IS_WRITE(frame->code) ? "write to" : "read from",
            faultaddr
        );
    }

    if(faultaddr < PAGE_SIZE)
        abandon_ship("Possible NULL dereference in ring 0 :(. Not proceeding.\n");
}

static int paging_identity_map_area(ptr base, ptr end)
{
    if(!bw_is_aligned(base, PAGE_SIZE))
        return 1;
    
    for(size_t i = base; i < end; i += PAGE_SIZE)
    {
        size_t pgtable_entry = i / PAGE_SIZE;
        size_t pgtable = pgtable_entry / 1024;
        if(pgtable > 0)
            TODO_FATAL(); /* Only first page table can be accessed for now. */
        pgtable_entry -= pgtable * 1024;

        PageTable *table = &g_pagetables[0];
        PageTableEntry *entry = &table->entries[pgtable_entry];

        pagetable_entry_set_frame_base(i, entry);
        pagetable_entry_set_present(true, entry);
        pagetable_entry_set_rw(true, entry);
    }

    return 0;
}

static int paging_identity_map(ptr base, size_t pages)
{
    return paging_identity_map_area(base, base + pages * PAGE_SIZE);
}

static void paging_enable()
{
    cpu_set_cr0(cpu_read_cr0() | CR0FLAG_PE | CR0FLAG_PG | CR0FLAG_WP);
}

void paging_init()
{
    pagedir_init(&g_pagedir);
    pagetable_init(&g_pagetables[0]);

    /* Mark the first page as absent. */
    pagetable_entry_set_frame_base(0, &g_pagetables[0].entries[0]);
    pagetable_entry_set_present(0, &g_pagetables[0].entries[0]);

    /* Identity map the first 1mb */
    paging_identity_map(4096, 255);

    /* Identity map the kernel */
    paging_identity_map_area(kinfo_get_kbase(), kinfo_get_kend());

    pagedir_entry_set_table_base(&g_pagetables[0], &g_pagedir.entries[0]);
    pagedir_entry_set_present(true, &g_pagedir.entries[0]);
    pagedir_entry_set_rw(true, &g_pagedir.entries[0]);

    pagedir_load(&g_pagedir);

    idt_register_isr(TRAP14, paging_isr);

    paging_enable();
    
    klog(INFO, "paging: Enabled paging.\n");
}
