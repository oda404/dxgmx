/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/module.h>
#include <dxgmx/panic.h>
#include <dxgmx/types.h>

_INIT bool kinit_stage2()
{
    module_init_builtins();

    if (!vfs_init())
        panic("Failed to initialize VFS!");

    while (1)
        cpu_suspend();

    return true;
}
