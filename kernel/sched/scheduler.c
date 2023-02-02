/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/scheduler.h>

#define KLOGF_PREFIX "sched: "

static Process* g_current_proc = NULL;

Process* sched_current_proc()
{
    return g_current_proc;
}

void sched_init()
{
    size_t proc_count = procm_proc_count();

    /* We should have 1 process, kinit_stage3 */
    if (proc_count != 1)
        panic("Scheduler found no PID 1!");

    KLOGF(INFO, "Starting PID 1.");
    g_current_proc = procm_next_queued_proc();
    procm_switch_ctx(g_current_proc);
}

void sched_yield()
{
    Process* proc = procm_next_queued_proc();

    while (proc->dead)
    {
        procm_try_kill_proc(g_current_proc, proc);
        proc = procm_next_queued_proc();
    }

    g_current_proc = proc;
    procm_switch_ctx(g_current_proc);
}
