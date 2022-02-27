/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/panic.h>
#include<dxgmx/kabort.h>
#include<dxgmx/klog.h>
#include<dxgmx/stack_trace.h>
#include<stdarg.h>

void panic(const char *lastmsg, ...)
{
    cmos_disable_nmi();
    interrupts_disable();

    klogln(FATAL, "");
    klogln(FATAL, "---[ uh-oh kernel panic :( ]---");
    stack_trace_dump();
    if(lastmsg)
    {
        va_list list;
        va_start(list, lastmsg);
        kvlogln(FATAL, lastmsg, list);
        va_end(list);
    }
    else
        klogln(FATAL, "nothing to say...");
    /* shit and cum my final message goodbye */
    kabort();
}
