/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_SCHED_H
#define _DXGMX_PROC_SCHED_H

#include <dxgmx/proc/proc.h>
#include <dxgmx/types.h>

typedef struct Scheduler
{
    const char* name;
    u32 priority;

    /* Pointer to an array of pointers :) */
    Process*** procs;
    size_t* proc_count;

    Process* (*current_proc)(struct Scheduler* sched);
    Process* (*next_proc)(struct Scheduler* sched);
    int (*reset)(struct Scheduler* sched);
} Scheduler;

#endif // !_DXGMX_PROC_SCHED_H
