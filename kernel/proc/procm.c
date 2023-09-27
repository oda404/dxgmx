/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/elf/elf.h>
#include <dxgmx/elf/elfloader.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "procm: "

static Process g_kernel_proc;
static Process* g_procs = NULL;
static size_t g_proc_count = 0;

static pid_t procm_next_available_pid()
{
    static pid_t running_pids = 1;

    // FIXME: actually handle such cases
    if (running_pids >= PID_MAX)
        return -1;

    return running_pids++;
}

static int procm_kill(Process* proc)
{
    proc_free(proc);

    for (Process* p = proc; p < g_procs + g_proc_count - 1; ++p)
        *p = *(p + 1);

    --g_proc_count;
    /* FIXME: we have a memory leak here since the process array is not
     shrinked, nor is this empty space accounted for when adding new processes
     to the pool. */
    return 0;
}

static int procm_add_proc_to_pool(Process* proc)
{
    /* Allocate new Process */
    Process* tmp = krealloc(g_procs, sizeof(Process) * (g_proc_count + 1));
    if (!tmp)
        return -ENOMEM;

    g_procs = tmp;
    ++g_proc_count;

    g_procs[g_proc_count - 1] = *proc;
    return 0;
}

int procm_replace_proc(
    const char* _USERPTR path,
    const char** argv,
    const char** envp,
    Process* actingproc)
{
    (void)argv;
    (void)envp;

    if (!path)
        return -EINVAL;

    /* When we replace a process, we keep everything from the old process except
     * it's path, and paging structure. Critically it's kernel stack is kept the
     * same. */

    Process newproc = {0};
    int st = proc_create_address_space(path, actingproc, &newproc);
    if (st < 0)
        return st;

    /* Change path */
    newproc.path = strdup(path);
    if (!newproc.path)
    {
        proc_free(&newproc);
        return -ENOMEM;
    }

    newproc.pid = actingproc->pid;
    newproc.fds = actingproc->fds;
    newproc.fd_count = actingproc->fd_count;
    newproc.kstack_top = actingproc->kstack_top;

    /* Load the context of the new process, so the old can be freed without any
     * worries. */
    proc_load_ctx(&newproc);
    proc_free(actingproc);
    *actingproc = newproc;

    /* At this point the old process is gone, and the new one is waiting to
     * run */
    sched_yield();
}

pid_t procm_spawn_proc(
    const char* _USERPTR path,
    const char** argv,
    const char** envp,
    Process* actingproc)
{
    (void)argv;
    (void)envp;

    Process newproc = {0};

    int st = proc_create_address_space(path, actingproc, &newproc);
    if (st < 0)
        return st;

    st = proc_create_kernel_stack(&newproc);
    if (st < 0)
    {
        proc_free(&newproc);
        return -ENOMEM;
    }

    /* Change path */
    newproc.path = strdup(path);
    if (!newproc.path)
    {
        proc_free(&newproc);
        return -ENOMEM;
    }

    newproc.pid = procm_next_available_pid();
    if (newproc.pid <= 0)
    {
        proc_free(&newproc);
        return -ENOSPC;
    }

    st = procm_add_proc_to_pool(&newproc);
    if (st < 0)
    {
        proc_free(&newproc);
        return st;
    }

    return newproc.pid;
}

_INIT pid_t procm_spawn_kernel_proc()
{
    g_kernel_proc.paging_struct = mm_get_kernel_paging_struct();
    return 0;
}

_INIT pid_t procm_spawn_init()
{
    const char* initpath = "/bin/main";
    if (procm_spawn_proc(initpath, NULL, NULL, procm_get_kernel_proc()) != 1)
        panic("Failed to spawn init: %s.", initpath);

    return 1;
}

int procm_mark_dead(int st, Process* proc)
{
    proc->exit_status = st;
    proc->dead = true;
    return 0;
}

void procm_switch_ctx(const Process* proc)
{
    ASSERT(proc);
    proc_load_ctx(proc);
    user_jump2user(proc->inst_ptr, proc->stack_ptr);
}

int procm_try_kill_proc(Process* actingproc, Process* targetproc)
{
    if (actingproc == targetproc)
    {
        /* pid 1 has exited. */
        if (g_proc_count == 1)
            panic("PID 1 returned %d.", targetproc->exit_status);

        return -EINVAL;
    }

    procm_kill(targetproc);
    return 0;
}

Process* procm_get_procs()
{
    return g_procs;
}

size_t procm_proc_count()
{
    return g_proc_count;
}

Process* procm_get_kernel_proc()
{
    return &g_kernel_proc;
}

Process* procm_get_proc_by_pid(pid_t pid)
{
    FOR_EACH_ELEM_IN_DARR (g_procs, g_proc_count, proc)
    {
        if (proc->pid == pid)
            return proc;
    }

    return NULL;
}

void sys_exit(int status)
{
    /* We can't straight up kill and free the curernt process, since that  would
     * mean freeing the process' kernel stack, which is also the stack we are
     * using right now. We deal with this by marking it as dead, and letting the
     * scheduler clean it up when it knows it safe to do so. */
    procm_mark_dead(status, sched_current_proc());
    sched_yield();
}

int sys_execve(const char* path, const char* argv[], const char* envp[])
{
    return procm_replace_proc(path, argv, envp, sched_current_proc());
}
