
#include<dxgmx/abandon_ship.h>
#include<stdlib.h>
#include<stdio.h>

void abandon_ship(const char *lastmsg)
{
    if(lastmsg)
        printf("abandoning ship: %s", lastmsg);
    /* shit and cum my final message goodbye */
    abort();
}
