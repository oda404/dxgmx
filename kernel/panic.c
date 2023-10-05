/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/kabort.h>
#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/stack_info.h>
#include <stdarg.h>

void panic(const char* lastmsg, ...)
{
    panic_arch_prepare();

    klogln(FATAL, "");
    klogln(FATAL, "--- uh-oh kernel panic :( ---");

    stack_info_dump_trace_lvl(FATAL);

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
