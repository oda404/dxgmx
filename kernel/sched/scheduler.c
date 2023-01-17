/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/panic.h>
#include <dxgmx/proc/procmanager.h>
#include <dxgmx/sched/scheduler.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/time.h>
#include <dxgmx/todo.h>
#include <dxgmx/userspace.h>

#define KLOGF_PREFIX "sched: "

static Process* g_current_proc = NULL;

Process* sched_current_proc()
{
    return g_current_proc;
}

void sched_init()
{
    Process* procs = procm_procs();
    size_t proc_count = procm_proc_count();

    /* We should have 1 process, kinit_stage3 */
    if (!procs || proc_count != 1)
        panic("Scheduler fount no PID 1!");

    g_current_proc = &procs[0];
    procm_switch_ctx(g_current_proc);
}

void sched_yield()
{
    Process* procs = procm_procs();
    size_t proc_count = procm_proc_count();

    FOR_EACH_ELEM_IN_DARR (procs, proc_count, proc)
    {
        if (proc->dead)
        {
            if (proc->pid == g_current_proc->pid)
            {
                /* We have one process and it's trying to die, I.E. pid 1 has
                 * returned */
                if (proc_count == 1)
                    panic(
                        "PID 1 returned %d, no more work to do.",
                        proc->exit_status);

                /* If the currenct process is trying to die, we skip it for now,
                since freeing it would mean freeing the kernel stack we are
                currently using */
                continue;
            }

            procm_kill(proc);
            continue;
        }
    }

    TODO_FATAL();
}
