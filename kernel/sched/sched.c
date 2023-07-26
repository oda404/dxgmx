/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/sched.h>

#define KLOGF_PREFIX "sched: "

static Process* g_procs;
static size_t g_proc_count;

static Process* g_current_proc = NULL;
static size_t g_current_proc_idx;

static Process* sched_next_proc()
{
    if (g_current_proc_idx >= g_proc_count)
        g_current_proc_idx = 0;

    return &g_procs[g_current_proc_idx++];
}

Process* sched_current_proc()
{
    return g_current_proc;
}

void sched_init()
{
    g_procs = procm_get_procs();
    g_proc_count = procm_proc_count();

    /* We should have 1 process, kinit_stage3 */
    Process* pid1 = procm_get_proc_by_pid(1);
    if (pid1 == NULL)
        panic("No PID 1!");

    g_current_proc = pid1;
    g_current_proc_idx = 0;

    KLOGF(INFO, "Starting PID 1.");
    procm_switch_ctx(g_current_proc);
}

void sched_yield()
{
    g_procs = procm_get_procs();
    g_proc_count = procm_proc_count();

    Process* next = sched_next_proc();
    while (next->dead)
    {
        procm_try_kill_proc(g_current_proc, next);
        next = sched_next_proc();
    }

    g_current_proc = next;
    procm_switch_ctx(g_current_proc);
}
