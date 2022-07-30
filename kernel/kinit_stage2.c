/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/module.h>
#include <dxgmx/panic.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/types.h>

bool kinit_stage2()
{
    if (syscalls_init() < 0)
        panic("Failed to setup system calls. Not proceeding!");

    module_init_builtins();

    if (!vfs_init())
        panic("Failed to initialize VFS!");

    while (1)
        cpu_suspend();

    return true;
}
