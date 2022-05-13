
#include <dxgmx/attrs.h>
#include <dxgmx/kabort.h>
#include <dxgmx/klog.h>
#include <dxgmx/stack_trace.h>
#include <stdarg.h>

void panic(const char* lastmsg, ...)
{
#ifdef _X86_
    __asm__ volatile("cli");
#endif

    klogln(FATAL, "");
    klogln(FATAL, "--- uh-oh kernel panic :( ---");

    stack_trace_dump();

    if (lastmsg)
    {
        va_list list;
        va_start(list, lastmsg);
        kvlogln(FATAL, lastmsg, list);
        va_end(list);
    }
    else
        klogln(FATAL, "nothing to say...");

    kabort();
}
