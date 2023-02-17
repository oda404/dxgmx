/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_PROCM_H
#define _DXGMX_PROC_PROCM_H

#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/proc_limits.h>
#include <posix/sys/types.h>

/* Load the 'first' PID 1 into memory and have it on standby, waiting for the
 * scheduler to start running. This is only a stub for the actual PID 1. This is
 * because the only way for us to read files from disk is through syscalls,
 * which we don't call from kernel space. With this method we have this PID 1
 * stub run as a user process, and let it load the actual PID 1 using the before
 * mentiond syscalls. Might have been linux-inspired */
pid_t procm_spawn_init();

/**
 * Spawn a new process
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

/**
 * Replace a running process with a fresly spawned version of another one.
 * Basically an execve.
 * 'path' path of the new process. Should not be NULL.
 * 'argv' Arguments.
 * 'envp' Environment variables.
 * 'proc' The process to be replaced. Should not be NULL.
 *
 * Returns:
 * doesn't return on success.
 */
int procm_replace_proc(
    const char* path,
    const char** argv,
    const char** envp,
    Process* actingproc);

/* Kill a process, freeing anything held by this process. */
int procm_kill(Process* proc);

/* Mark a process as dead, letting it be reaped by the scheduler. */
int procm_mark_dead(int st, Process* proc);

/* Start executing this process. */
_ATTR_NORETURN void procm_switch_ctx(Process* proc);

size_t procm_proc_count();

/**
 * Try to kill a process. This function can fail if we are trying to kill the
 * acting process.
 *
 * No NULL pointers should be passed to this function.
 *
 * 'actingproc' Acting process.
 * 'targetproc' The process to be kill.
 *
 * Returns:
 * 0 on sucess.
 * -EINVAL if targetproc == actingproc.
 */
int procm_try_kill_proc(Process* actingproc, Process* targetproc);

/**
 * Get the next process that is queued to run.
 *
 * Returns:
 * A non-NULL Process*.
 */
Process* procm_next_queued_proc();

#endif // !_DXGMX_PROC_PROCM_H
