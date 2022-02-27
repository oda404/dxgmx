/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/types.h>
#include<dxgmx/attrs.h>
#include<dxgmx/cpu.h>

_INIT bool kinit_stage2()
{
    while(1)
        cpu_suspend();

    return true;
}
