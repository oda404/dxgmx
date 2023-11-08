/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagefault.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/user.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "pagefault: "

#define PAGEFAULT_VERBOSE 1

#define ACTION_TO_MSG(_action, _msg)                                           \
    if (_action == PAGEFAULT_ACTION_EXEC)                                      \
        _msg = "x";                                                            \
    else if (_action == PAGEFAULT_ACTION_WRITE)                                \
        _msg = "w";                                                            \
    else                                                                       \
        _msg = "r";

static void pagefault_handle_prot_fault(
    ptr faultaddr, u8 ring, PageFaultAction action, ptr ip)
{
    if (ring == 0)
    {
        const char* action_msg = NULL;
        ACTION_TO_MSG(action, action_msg);

        panic(
            "Kernel page protection violation @ 0x%p (%s)  ip(0x%p)!",
            (void*)faultaddr,
            action_msg,
            (void*)ip);
    }
    else
    {
        // FIXME: implement actual process protection violation
        const char* action_msg = NULL;
        ACTION_TO_MSG(action, action_msg);

        panic(
            "User page protection violation at: 0x%p (%s), ip: 0x%p",
            (void*)faultaddr,
            action_msg,
            (void*)ip);
    }
}

static void pagefault_handle_absent_kernel_heap(ptr faultaddr)
{
    const ptr aligned_faultaddr = bytes_align_down64(faultaddr, PAGESIZE);
    const ptr frame = aligned_faultaddr - kimg_map_offset();

    /* We may have other available page frames, other than the one we requested,
     * but using those will break the promise that the kernel heap is contiguous
     * in both virtual & physical memory. */
    int st = falloc_one_at(frame);
    if (st < 0)
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
    st = mm_map_page(
        aligned_faultaddr, frame, PAGE_RW, mm_get_kernel_paging_struct());
    if (st < 0)
        panic("Kernel out of virtual memory!");

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

static void handle_absent_user(ptr faultaddr)
{
    panic("Page fault in ring 3 @ 0x%p!", (void*)faultaddr);
}

static void pagefault_handle_absent(ptr faultaddr, u8 ring)
{
    if (ring == 0)
    {
        if (kmalloc_owns_va(faultaddr))
            pagefault_handle_absent_kernel_heap(faultaddr);
        else
            pagefault_handle_absent_kernel_bogus(faultaddr);
    }
    else
    {
        handle_absent_user(faultaddr);
    }
}

ptr pagefault_handle(
    ptr faultaddr,
    ptr ip,
    u8 ring,
    PageFaultReason reason,
    PageFaultAction action)
{
    const bool user_access =
        ip >= kimg_useraccess_start() && ip < kimg_useraccess_end();

#ifdef PAGEFAULT_VERBOSE
    const char* action_msg = NULL;
    ACTION_TO_MSG(action, action_msg);

    const char* reason_msg = NULL;
    if (reason == PAGEFAULT_REASON_ABSENT)
        reason_msg = "absent";
    else
        reason_msg = "prot. fault";

    KLOGF(
        DEBUG,
        "Fault @ 0x%p (%s/%s) cpl(%d) ip(0x%p)%s%s",
        (void*)faultaddr,
        action_msg,
        reason_msg,
        ring,
        (void*)ip,
        user_access ? " (.useraccess)" : "",
        kmalloc_owns_va(faultaddr) ? " (.kheap)" : "");
#endif // PAGEFAULT_VERBOSE

    if (reason == PAGEFAULT_REASON_PROT_FAULT)
    {
        if (user_access)
            return (ptr)user_access_fault_stub;

        pagefault_handle_prot_fault(faultaddr, ring, action, ip);
    }
    else
    {
        /* We might user fault when trying to access an unmapped kheap page. In
         * which case we want to map it. */
        if (user_access && !kmalloc_owns_va(faultaddr))
            return (ptr)user_access_fault_stub;

        pagefault_handle_absent(faultaddr, ring);
    }

    return ip;
}
