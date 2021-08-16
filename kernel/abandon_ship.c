/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/abandon_ship.h>
#include<dxgmx/kabort.h>
#include<dxgmx/klog.h>
#include<stdarg.h>

void abandon_ship(const char *lastmsg, ...)
{
    klog(KLOG_FATAL, "Abandoning ship");
    if(lastmsg)
    {
        klog(KLOG_FATAL, ": ");
        va_list list;
        va_start(list, lastmsg);
        kvlog(KLOG_FATAL, lastmsg, list);
        va_end(list);
    }
    /* shit and cum my final message goodbye */
    kabort();
}
