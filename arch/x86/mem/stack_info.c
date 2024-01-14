/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/cpu.h>
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/kimg.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/stack_info.h>

void stack_info_dump_trace()
{
    stack_info_dump_trace_lvl(DEBUG);
}

void stack_info_dump_trace_lvl(KLogLevel lvl)
{
    const StackFrame* frame = (StackFrame*)cpu_read_bp();

    klogln(lvl, "Call stack backtrace:");
    if (!frame)
    {
        klogln(lvl, "- ebp is NULL, aborting stack trace!");
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
            klogln(lvl, "- next ip is in userspace, aborting stack trace!");
            return;
        }

        char buf[FUNC_BUF_MAX + 1] = "???";

        ptr offset = 0;
        ksyms_get_symbol_name(frame->instptr - 1, &offset, buf, FUNC_BUF_MAX);

        klogln(
            lvl,
            "  0x%p - [%s] + 0x%zx",
            (void*)(frame->instptr - 1),
            buf,
            offset);

        frame = (StackFrame*)frame->baseptr;

        ++frames;

        if (frames == FRAMES_UNWIND_MAX)
            klogln(lvl, "- Hit FRAMES_UNWIND_MAX, truncated stack strace!");
    }
}

void stack_info_dump_limits(const char* func, int line)
{
    const Process* proc = procm_sched_current_proc();
    const size_t esp = cpu_read_sp();

    size_t rem;
    if (proc)
        rem = esp - (proc->kstack_top - PROC_KSTACK_SIZE);
    else
        rem = esp - kimg_kstack_bot();

    klogln(
        DEBUG,
        "kstack: @%s():%d is %zu bytes away from overflowing.",
        func,
        line,
        rem);
}
