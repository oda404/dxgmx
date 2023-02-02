/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/proc/procmanager.h>
#include <dxgmx/sched/scheduler.h>
#include <dxgmx/syscalls.h>

int syscall_execve(const char* path, const char* argv[], const char* envp[])
{
    return procm_replace_proc(path, argv, envp, sched_current_proc());
}