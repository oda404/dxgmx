/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/abandon_ship.h>
#include<dxgmx/kabort.h>
#include<dxgmx/kprintf.h>
#include<stdarg.h>

void abandon_ship(const char *lastmsg, ...)
{
    kprintf("Abandoning ship ");
    if(lastmsg)
    {
        va_list list;
        va_start(list, lastmsg);
        vkprintf(lastmsg, list);
        va_end(list);
    }
    /* shit and cum my final message goodbye */
    kabort();
}
