/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/cpu.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/pagefault.h>

#define KLOGF_PREFIX "pagefault: "

#define PAGEFAULT_VERBOSE 1

#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))
#define PAGEFAULT_IS_EXEC(x) (x & (1 << 4))

static ptr g_kheap_start;

/* Handler for a page protection violation fault. */
static void
pagefault_handle_prot_viol(ptr faultaddr, const InterruptFrame* frame)
{
    const u8 cpl = frame->cs & 3;
    ASSERT(cpl == 0 || cpl == 3);

    if (cpl == 0)
    {
        const char* action_msg = NULL;
        if (PAGEFAULT_IS_EXEC(frame->code))
            action_msg = "x";
        else if (PAGEFAULT_IS_WRITE(frame->code))
            action_msg = "w";
        else
            action_msg = "r";

        panic(
            "Kernel page protection violation @ 0x%p (%s)  ip(0x%p)!",
            (void*)faultaddr,
            action_msg,
            (void*)frame->eip);
    }
    else
    {
        // FIXME: implement actual process protection violation
        const char* action_msg = NULL;
        if (PAGEFAULT_IS_EXEC(frame->code))
            action_msg = "exec";
        else if (PAGEFAULT_IS_WRITE(frame->code))
            action_msg = "write";
        else
            action_msg = "read";

        panic(
            "User page protection violation at: 0x%p (%s), ip: 0x%p",
            (void*)faultaddr,
            action_msg,
            (void*)frame->eip);
    }
}

static void handle_absent_user(ptr faultaddr, InterruptFrame* frame)
{
    (void)faultaddr;
    (void)frame;
    panic("Page fault in ring 3 :(");
}

static void pagefault_handle_absent_kernel_heap(ptr faultaddr)
{
    const ptr aligned_faultaddr = bytes_align_down64(faultaddr, PAGESIZE);
    ++g_nested_pagefaults;

    ptr frame = falloc_one_user();
    if (!frame)
        panic("Kernel out of memory!");

    /** mm_new_page may call kmalloc which is very risky, as we are already in a
     * spot where we don't have *some* memory mapped. Ideally we could keep
     * track of how many recursive page faults happen and maybe have some sort
     * of backup cache of PageTables as a last case scenario. This works right
     * now, as the allocator implementation puts the biggest chunks of
     * memory (the chunks used for allocating stuff >= 128bytes) at the
     * start of the heap, which is most likely mapped. */

    /* !!! NOTE FOR WHEN THIS BITES ME IN THE ASS LATER: If this fails
     * the kernel will panic with a "Kernel out of memory" :)
     */
    st = mm_new_page(
        aligned_faultaddr,
        frame,
        PAGE_R | PAGE_W,
        mm_get_kernel_paging_struct());

    // Kernel heap pages are zereod out
    memset((void*)aligned_faultaddr, 0, PAGESIZE);
}

static void pagefault_handle_absent_kernel_bogus(ptr faultaddr)
{
    /* We know that the kernel image is mapped, the kernel heap
     * comes after the kernel image and any other faults should have
     * cpl == 3, so this is the kernel doing something  it's not supposed to do.
     * But we can still get some insight into what it's trying to do. */

    if (faultaddr < PAGESIZE)
        panic("Possible NULL dereference in ring 0 :(");

    panic("Kernel tried mapping a weird page: 0x%p", (void*)faultaddr);
}

/* Handler for an absent page fault. */
static void pagefault_handle_absent(ptr faultaddr, InterruptFrame* frame)
{
    const u8 cpl = frame->cs & 3;
    ASSERT(cpl == 0 || cpl == 3);

    if (cpl == 0)
    {
        if (faultaddr >= g_kheap_start)
            pagefault_handle_absent_kernel_heap(faultaddr);
        else
            pagefault_handle_absent_kernel_bogus(faultaddr);
    }
    else
    {
        handle_absent_user(faultaddr, frame);
    }
}

static void pagefault_isr(InterruptFrame* frame)
{
    const ptr faultaddr = cpu_read_cr2();
    const bool user_access = frame->eip >= kimg_useraccess_start() &&
                             frame->eip < kimg_useraccess_end();

#if PAGEFAULT_VERBOSE == 1
    const char* action_msg = NULL;
    if (PAGEFAULT_IS_EXEC(frame->code))
        action_msg = "x";
    else if (PAGEFAULT_IS_WRITE(frame->code))
        action_msg = "w";
    else
        action_msg = "r";

    const char* result_msg = NULL;
    if (PAGEFAULT_IS_PROT_VIOL(frame->code))
        result_msg = "prot. fault";
    else
        result_msg = "absent";

    KLOGF(
        DEBUG,
        "Fault @ 0x%p (%s/%s) cpl(%d) ip(0x%p)%s%s",
        (void*)faultaddr,
        action_msg,
        result_msg,
        frame->cs & 3,
        (void*)frame->eip,
        user_access ? " (.useraccess)" : "",
        faultaddr >= g_kheap_start ? " (.kheap)" : "");
#endif // PAGEFAULT_VERBOSE == 1

    if (PAGEFAULT_IS_PROT_VIOL(frame->code))
    {
        if (user_access)
        {
            frame->eip = (ptr)user_access_fault_stub;
            return;
        }

        pagefault_handle_prot_viol(faultaddr, frame);
    }
    else
    {
        /* We might user fault when trying to access an unmapped kheap page. In
         * which case we want to map it. */
        if (user_access && faultaddr < g_kheap_start)
        {
            frame->eip = (ptr)user_access_fault_stub;
            return;
        }

        pagefault_handle_absent(faultaddr, frame);
    }
}

void pagefault_setup_isr()
{
    g_kheap_start = kimg_vaddr() + kimg_size();
    idt_register_trap_isr(TRAP_PAGEFAULT, 0, pagefault_isr);
}
