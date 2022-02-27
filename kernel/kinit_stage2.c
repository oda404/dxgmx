/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/types.h>
#include<dxgmx/attrs.h>
#include<dxgmx/panic.h>
#include<dxgmx/cpu.h>
#include<dxgmx/vfs.h>

_INIT bool kinit_stage2()
{
    if(!vfs_init())
        panic("Failed to initialize VFS!");

    while(1)
        cpu_suspend();

    return true;
}
