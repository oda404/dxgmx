/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/cpu.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/panic.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/mm.h>
#include <dxgmx/x86/pagefault.h>

#define KLOGF_PREFIX "pagefault: "

#define PAGEFAULT_VERBOSE 1

#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))
#define PAGEFAULT_IS_EXEC(x) (x & (1 << 4))

/* Handler for a page protection violation fault. */
static void
handle_page_prot_violation_fault(ptr faultaddr, const InterruptFrame* frame)
{
    const u8 cpl = frame->cs & 3;
    ASSERT(cpl == 0 || cpl == 3);

    if (cpl == 0)
    {
        const char* msg = NULL;
        if (PAGEFAULT_IS_EXEC(frame->code))
            msg = "exec from";
        else if (PAGEFAULT_IS_WRITE(frame->code))
            msg = "write to";
        else
            msg = "read from";

        panic(
            "Page protection violation: tried to %s 0x%p in ring 0.",
            msg,
            (void*)faultaddr);
    }
    else
    {
        TODO_FATAL();
    }
}

static void handle_absent_kernel_heap_page(ptr faultaddr)
{
    ptr required_frame =
        bytes_align_down64(faultaddr, PAGESIZE) - kimg_map_offset();

    int st = falloc_one_at(required_frame);

    /* We may have other available page frames, other than the one we requested,
     * but using those will break the promise that the kernel heap is "higher
     * half mapped". */
    if (st < 0)
        panic("Kernel out of memory!");

    pdpt_t* kpdpt = mm_kernel_pdpt();

    PageTable* pt = pt_from_vaddr_abs(faultaddr, kpdpt);
    if (!pt)
    {
        PageDirectoryEntry* pde = pde_from_vaddr_abs(faultaddr, kpdpt);
        ASSERT(pde);

        /* Calling kmalloc here is very risky, as we are already in a spot where
         * we don't have *some* memory mapped. Ideally we could keep track of
         * how many recursive page faults happen and maybe have some sort of
         * backup cache of PageTables as a last case scenario. This works right
         * now, as the allocator implementation puts the biggest chunks of
         * memory (the chunks used for allocating stuff >= 128bytes) at the
         * start of the heap, which is most likely mapped. */

        /* !!! NOTE FOR WHEN THIS BITES ME IN THE ASS LATER: If this kmalloc
         * call fails the kernel will panic with a "Kernel out of memory" :) */

        pt = kmalloc_aligned(sizeof(PageTable), PAGESIZE);
        pagetable_init(pt);

        pde_set_table_base(mm_kvaddr2paddr((ptr)pt), pde);
        pde->present = true;
        pde->writable = true;
        pde->exec_disable = true;
    }

    PageTableEntry* pte = pte_from_vaddr(faultaddr, pt);
    ASSERT(pte);

    pte_set_frame_base(required_frame, pte);
    pte->exec_disable = true;
    pte->present = true;
    pte->writable = true;

    mm_tlb_flush_single(faultaddr);
}

/* Handler for an absent page fault. */
static void handle_absent_pagefault(ptr faultaddr, InterruptFrame* frame)
{
    const u8 cpl = frame->cs & 3;
    ASSERT(cpl == 0 || cpl == 3);

    if (cpl == 0)
    {
        if (faultaddr >= kimg_vaddr() + kimg_size())
        {
            handle_absent_kernel_heap_page(faultaddr);
        }
        else
        {
            /* We know that the kernel image is mapped, the kernel heap
             * comes after the kernel image and any other faults should have
             * cpl == 3, so this is the kernel doing something
             * it's not supposed to do. */
            panic("Kernel tried mapping a weird page: 0x%p", (void*)faultaddr);
        }
    }
    else
    {
        TODO_FATAL();
    }
}

static void pagefault_isr(InterruptFrame* frame)
{
    const ptr faultaddr = cpu_read_cr2();

#if PAGEFAULT_VERBOSE == 1
    const char* msg = NULL;
    if (PAGEFAULT_IS_PROT_VIOL(frame->code))
    {
        if (PAGEFAULT_IS_EXEC(frame->code))
            msg = "exec";
        else if (PAGEFAULT_IS_WRITE(frame->code))
            msg = "write";
        else
            msg = "read";
    }
    else
    {
        msg = "absent";
    }

    KLOGF(
        DEBUG,
        "Fault at: 0x%p (%s), cpl: %d.",
        (void*)faultaddr,
        msg,
        frame->cs & 3);
#endif // PAGEFAULT_VERBOSE == 1

    if ((frame->cs & 3) == 0 && faultaddr < PAGESIZE)
        panic("Possible NULL dereference in ring 0 :(");

    if (PAGEFAULT_IS_PROT_VIOL(frame->code))
        handle_page_prot_violation_fault(faultaddr, frame);
    else
        handle_absent_pagefault(faultaddr, frame);
}

void pagefault_setup_isr()
{
    idt_register_isr(TRAP14, pagefault_isr);
}
