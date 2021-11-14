/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/paging.h>
#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/pagetable.h>
#include<dxgmx/x86/pagedir.h>
#include<dxgmx/x86/pagedir_ptrtable.h>
#include<dxgmx/compiler_attrs.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/klog.h>
#include<dxgmx/cpu.h>
#include<dxgmx/panic.h>
#include<dxgmx/bitwise.h>
#include<dxgmx/math.h>
#include<dxgmx/todo.h>
#include<dxgmx/attrs.h>
#include<dxgmx/string.h>

static _ATTR_ALIGNED(PAGE_SIZE) PageDirectoryPointerTable g_pdpt;
static _ATTR_ALIGNED(PAGE_SIZE) PageDirectory g_pagedir;
static _ATTR_ALIGNED(PAGE_SIZE) PageTable g_pagetable;

#define KLOGF(lvl, fmt, ...) klogln(lvl, "paging: " fmt, ##__VA_ARGS__)

static void paging_isr(
    const InterruptFrame *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))
#define PAGEFAULT_IS_EXEC(x) (x & (1 << 4))

    ptr faultaddr = cpu_read_cr2();

    if(faultaddr < PAGE_SIZE)
        panic("Possible NULL dereference in ring 0 :(. Not proceeding.");

    if(PAGEFAULT_IS_PROT_VIOL(frame->code))
    {
        char msg[10];
        if(PAGEFAULT_IS_EXEC(frame->code))
            strcpy(msg, "exec from");
        else if(PAGEFAULT_IS_WRITE(frame->code))
            strcpy(msg, "write to");
        else
            strcpy(msg, "read from");

        panic("Page protection violation: tried to %s 0x%p. Not proceeding.", msg, (void*)faultaddr);
    }
    else
        TODO_FATAL();
}

_INIT static int paging_identity_map_area(ptr base, ptr end)
{
    if(base % PAGE_SIZE != 0)
        return 1;
    
    for(size_t i = base; i < end; i += PAGE_SIZE)
    {
        PageTableEntry *entry = paging_pte_from_vaddr(i);

        pte_set_frame_base(i, entry);
        entry->present = true;
        entry->rw = true;
    }

    return 0;
}

extern u8 _kernel_base[];
extern u8 _kernel_end[];

_INIT void paging_init()
{
    if(!cpu_has_feature(CPU_PAE))
        panic("CPU doesn't support PAE, we don't support that. Not proceeding.");

    KLOGF(INFO, "Setting up 3-level paging with PAE.");

    pdpt_init(&g_pdpt);
    pagedir_init(&g_pagedir);
    pagetable_init(&g_pagetable);
    /* Enable page address extension for the execution disable bit and
    disable page size extension as we don't use it. */
    cpu_write_cr4((cpu_read_cr4() | CR4_PAE) & ~(CR4_PSE));
    cpu_write_msr(cpu_read_msr(MSR_EFER) | EFER_NXE, MSR_EFER);

    /* Mark the first page as absent so we can't do shit like *(NULL). */
    g_pagetable.entries[0].frame_base = 0;
    g_pagetable.entries[0].present = false;

    pde_set_table_base((ptr)&g_pagetable, &g_pagedir.entries[0]);
    g_pagedir.entries[0].present = true;
    g_pagedir.entries[0].rw = true;

    pdpte_set_pagedir_base((ptr)&g_pagedir, &g_pdpt.entries[0]);
    g_pdpt.entries[0].present = true;

    paging_identity_map_area(PAGE_SIZE, MIB);
    paging_identity_map_area((ptr)_kernel_base, (ptr)_kernel_end);

    /* Flush pdpt. */
    cpu_write_cr3((u32)&g_pdpt);

    idt_register_isr(TRAP14, paging_isr);

    /* Enable paging. */
    cpu_write_cr0(cpu_read_cr0() | CR0_PG | CR0_WP);
}

PageTableEntry *paging_pte_from_vaddr(ptr vaddr)
{
    size_t pte_idx = vaddr / PAGE_SIZE;
    size_t pt_idx = pte_idx / 512;
    if(pt_idx > 0)
        TODO_FATAL(); /* Only first page table can be accessed for now. */
    pte_idx -= pt_idx * 512;

    PageTable *table = &g_pagetable;
    return &table->entries[pte_idx];
}

void paging_flush_tlb_entries(ptr vaddr)
{
    /*FIXEME: check for i386 CPUs that don't support invlpg ?
    Though, if a CPU supports the mandatory PAE it should also support invlpg ? */
    __asm__ volatile("invlpg (%0)": : "a"(vaddr));
}
