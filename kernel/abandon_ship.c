/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/abandon_ship.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/kprintf.h>

void abandon_ship(const char *lastmsg)
{
    if(lastmsg)
        kprintf("abandoning ship: %s", lastmsg);
    /* shit and cum my final message goodbye */
    kabort();
}
