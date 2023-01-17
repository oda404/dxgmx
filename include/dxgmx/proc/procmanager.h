/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_PROCM_H
#define _DXGMX_PROC_PROCM_H

#include <dxgmx/proc/proc.h>
#include <posix/sys/types.h>

/* Load the 'first' PID 1 into memory and have it on standby, waiting for the
 * scheduler to start running. This is only a stub for the actual PID 1. This is
 * because the only way for us to read files from disk is through syscalls,
 * which we don't call from kernel space. With this method we have this PID 1
 * stub run as a user process, and let it load the actual PID 1 using the before
 * mentiond syscalls. Might have been linux-inspired */
pid_t procm_spawn_init();

/* Create a new process from 'path'. The process doesn't actually run until the
 * scheduler decides to run it.
 * Returns negative on errors, or the process id. */
pid_t procm_spawn_proc(const char* path);

/* Kill a process, freeing anything held by this process. */
int procm_kill(Process* proc);

/* Mark a process as dead, letting it be reaped by the scheduler. */
int procm_mark_dead(int st, Process* proc);

/* Preapres the system for a context switch to this process. */
int procm_load_ctx(Process* proc);

/* Start executing this process. */
_ATTR_NORETURN void procm_switch_ctx(Process* proc);

/* Returns the Process object associated with 'pid'. NULL if none. */
Process* procm_proc_from_pid(pid_t pid);

Process* procm_procs();
size_t procm_proc_count();

#endif // !_DXGMX_PROC_PROCM_H
