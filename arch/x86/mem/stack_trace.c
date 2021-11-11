
#include<dxgmx/stack_trace.h>
#include<dxgmx/cpu.h>
#include<dxgmx/klog.h>

void stack_trace_dump()
{
    const StackFrame *frame = NULL;
#if defined(_X86_)
    frame = (StackFrame*)cpu_read_ebp();
#elif defined(_X86_64_)
    frame = (StackFrame*)cpu_read_rbp();
#endif

    klogln(INFO, "Call stack backtrace:");
    if(!frame)
    {
        klogln(WARN, "Current ebp is 0, something has gone horribly wrong lmao.");
        return;
    }

    while(frame)
    {
        klogln(INFO, "  0x%p - ???", (void*)frame->instptr);
        frame = (StackFrame*)frame->baseptr;
    }
}