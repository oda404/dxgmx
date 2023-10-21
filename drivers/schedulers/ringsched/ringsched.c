/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/module.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/proc/sched.h>

static size_t g_current_proc_idx;

static Process* ringsched_next_proc(Scheduler* sched)
{
    size_t newidx = g_current_proc_idx + 1;
    while (true) // Possible infinite loop ? idk i'm drunk
    {
        if (newidx >= *sched->proc_count)
        {
            newidx = 0;
            continue;
        }

        if ((*sched->procs)[newidx] == NULL)
        {
            ++newidx;
            continue;
        }

        g_current_proc_idx = newidx;
        return (*sched->procs)[newidx];
    }
}

static Process* ringsched_current_proc(Scheduler* sched)
{
    return (*sched->procs)[g_current_proc_idx];
}

static int ringsched_reset(Scheduler*)
{
    g_current_proc_idx = 0;
    return 0;
}

static Scheduler g_ringsched = {
    .name = "ringscheduler",
    .priority = 100,
    .next_proc = ringsched_next_proc,
    .current_proc = ringsched_current_proc,
    .reset = ringsched_reset};

static int ringsched_main()
{
    return procm_sched_register(&g_ringsched);
}

static int ringsched_exit()
{
    return procm_sched_unregister(&g_ringsched);
}

MODULE g_ringsched_module = {
    .name = "ringsched",
    .stage = MODULE_STAGE3,
    .main = ringsched_main,
    .exit = ringsched_exit};
