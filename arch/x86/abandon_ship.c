
#include<dxgmx/abandon_ship.h>
#include<dxgmx/kstdlib.h>
#include<dxgmx/kstdio.h>

void abandon_ship(const char *lastmsg)
{
    if(lastmsg)
        kprintf("abandoning ship: %s", lastmsg);
    /* shit and cum my final message goodbye */
    kabort();
}
