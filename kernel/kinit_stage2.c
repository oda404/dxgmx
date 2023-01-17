/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/module.h>
#include <dxgmx/panic.h>
#include <dxgmx/proc/procmanager.h>
#include <dxgmx/sched/scheduler.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/types.h>

_ATTR_NORETURN bool kinit_stage2()
{
    if (syscalls_init() < 0)
        panic("Failed to setup system calls. Not proceeding!");

    module_init_builtins();

    if (vfs_init() < 0)
        panic("Failed to initialize VFS!");

    /* Spawn PID 1. Here, we only load the binary in memory and have it ready to
     * run, waiting for the scheduler to kick in. For details on how this binary
     * works check out kernel/kinit_stage3.c */
    if (procm_spawn_init() != 1)
        panic("Failed to spawn PID 1!");

    /* Let it rip */
    sched_init();
}
