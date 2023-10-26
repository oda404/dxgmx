/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_PROCM_H
#define _DXGMX_PROC_PROCM_H

#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/proc/sched.h>

#define PID_MAX __INT32_MAX__

int procm_init();

/* Load the 'first' PID 1 into memory and have it on standby, waiting for the
 * scheduler to start running. This is only a stub for the actual PID 1. This is
 * because the only way for us to read files from disk is through syscalls,
 * which we don't call from kernel space. With this method we have this PID 1
 * stub run as a user process, and let it load the actual PID 1 using the before
 * mentiond syscalls. Might have been linux-inspired */
pid_t procm_spawn_init();

/**
 * Spawn the kernel acting process.
 */
pid_t procm_spawn_kernel_proc();

/**
 * Spawn a new process. The spawned process is added to the process queue and
 * will run when the scheduler decides to.
 *
 * 'path' path of the new process. Should not be NULL.
 * 'argv' Arguments.
 * 'envp' Environment variables.
 * 'proc' Acting process. Should not be NULL.
 *
 * Returns:
 * 0 on success.
 * -EINVAL on invalid arguments.
 * -ENOMEM on out of memory.
 * -ENOSPC if no pid could be allocated.
 */
pid_t procm_spawn_proc(
    const char* path,
    const char** argv,
    const char** envp,
    Process* actingproc);

/* Mark a process as dead, letting it be reaped by the scheduler. */
int procm_mark_dead(int st, Process* proc);

Process* procm_get_kernel_proc();

int procm_sched_register(Scheduler* sched);
int procm_sched_unregister(Scheduler* sched);

_ATTR_NORETURN void procm_sched_start();
Process* procm_sched_current_proc();
void procm_sched_yield();

#endif // !_DXGMX_PROC_PROCM_H
