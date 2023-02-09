

#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/syscalls.h>

void syscall_exit(int status)
{
    /* We can't straight up kill and free the curernt process, since that would
     * mean freeing the process' kernel stack, which is also the stack we are
     * using right now. We deal with this by marking it as dead, and letting the
     * scheduler clean it up when it knows it safe to do so. */
    procm_mark_dead(status, sched_current_proc());

    sched_yield();
}

int syscall_execve(const char* path, const char* argv[], const char* envp[])
{
    return procm_replace_proc(path, argv, envp, sched_current_proc());
}
