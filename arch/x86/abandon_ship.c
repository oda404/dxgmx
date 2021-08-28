/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/kabort.h>
#include<dxgmx/klog.h>
#include<stdarg.h>

void abandon_ship(const char *lastmsg, ...)
{
    cmos_disable_nmi();
    interrupts_disable();

    klog(KLOG_FATAL, "---[ start of abandon ship ]---\n");
    if(lastmsg)
    {
        va_list list;
        va_start(list, lastmsg);
        kvlog(KLOG_FATAL, lastmsg, list);
        va_end(list);
    }
    else
        klog(KLOG_FATAL, "nothing to say...\n");
    klog(KLOG_FATAL, "---[ end of abandon ship ]---\n");
    /* shit and cum my final message goodbye */
    kabort();
}
