/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/cpu.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/stack_info.h>

void stack_info_dump_trace()
{
    const StackFrame* frame = NULL;
#if defined(CONFIG_X86)
    frame = (StackFrame*)cpu_read_ebp();
#elif defined(CONFIG_X86_64)
    frame = (StackFrame*)cpu_read_rbp();
#endif

    klogln(INFO, "Call stack backtrace:");
    if (!frame)
    {
        klogln(ERR, "- ebp is NULL, aborting stack trace!");
        return;
    }

#define FRAMES_UNWIND_MAX 20
#define FUNC_BUF_MAX 49

    size_t frames = 0;
    while (frame && frames < FRAMES_UNWIND_MAX)
    {
        if (frame->instptr < kimg_vaddr())
        {
            /* This pointer belongs to the userspace */
            klogln(ERR, "- next ip is in userspace, aborting stack trace!");
            return;
        }

        char buf[FUNC_BUF_MAX + 1] = "???";

        ptr offset = 0;
        ksyms_get_symbol_name(frame->instptr - 1, &offset, buf, FUNC_BUF_MAX);

        klogln(
            INFO,
            "  0x%p - [%s] + 0x%X",
            (void*)(frame->instptr - 1),
            buf,
            offset);

        frame = (StackFrame*)frame->baseptr;

        ++frames;

        if (frames == FRAMES_UNWIND_MAX)
            klogln(WARN, "- Hit FRAMES_UNWIND_MAX, truncated stack strace!");
    }
}

void stack_info_dump_limits(const char* func, int line)
{
    Process* proc = sched_current_proc();
    size_t esp = cpu_read_esp();
    size_t rem;
    if (proc)
    {
        rem = esp - (proc->kstack_top - PROC_KSTACK_SIZE);
    }
    else
    {
        size_t stack_size = kimg_kstack_top() - kimg_kstack_bot();
        rem = esp - (kimg_kstack_top() - stack_size);
    }

    klogln(
        DEBUG,
        "kstack: @%s():%d is %d bytes away from overflowing.",
        func,
        line,
        rem);
}
