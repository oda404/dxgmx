
#include<dxgmx/cpu.h>

int kinit_stage2()
{
    while(1)
        cpu_suspend();

    return 0;
}
