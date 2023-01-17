/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/proc/procmanager.h>
#include <dxgmx/sched/scheduler.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/todo.h>

void syscall_exit(int status)
{
    /* We can't straight up kill and free the curernt process, since that would
     * mean freeing the process' kernel stack, which is also the stack we are
     * using right now. We deal with this by marking it as dead, and letting the
     * scheduler clean it up when it knows it safe to do so. */
    procm_mark_dead(status, sched_current_proc());

    sched_yield();
}
