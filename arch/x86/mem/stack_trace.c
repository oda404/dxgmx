/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/cpu.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/stack_trace.h>

void stack_trace_dump()
{
    const StackFrame* frame = NULL;
#if defined(_X86_)
    frame = (StackFrame*)cpu_read_ebp();
#elif defined(_X86_64_)
    frame = (StackFrame*)cpu_read_rbp();
#endif

    klogln(INFO, "Call stack backtrace:");
    if (!frame)
    {
        klogln(ERR, "- ebp is NULL, aborting stack trace dump!");
        return;
    }

#define FRAMES_UNWIND_MAX 20
#define FUNC_BUF_MAX 49

    size_t frames = 0;
    while (frame && frames < FRAMES_UNWIND_MAX)
    {
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
