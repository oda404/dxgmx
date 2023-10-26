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
#include <dxgmx/proc/sched.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "procm: "

static Process g_kernel_proc;

static Process** g_procs;
static size_t g_proc_size;
static size_t g_last_free_proc_idx;
/* How many actual processes are allocated (some of them may be zombies) */
static size_t g_proc_count;

static Scheduler** g_schedulers;
static size_t g_scheduler_count;
static Scheduler* g_active_sched;

static pid_t procm_available_pid()
{
    const size_t start_point = g_last_free_proc_idx;
    while (g_last_free_proc_idx < g_proc_size && g_procs[g_last_free_proc_idx])
    {
        // overflow check
        if (++g_last_free_proc_idx == start_point)
            return -EOVERFLOW;
    }

    return g_last_free_proc_idx++ + 1;
}

static int procm_add_proc_to_pool(Process* proc)
{
    size_t pid_idx = (size_t)proc->pid - 1;
    ASSERT(pid_idx <= g_proc_size);

#define ALLOC_STEP 16
    if (pid_idx == g_proc_size)
    {
        const size_t prevsize = g_proc_size * sizeof(Process*);
        const size_t stepsize = ALLOC_STEP * sizeof(Process*);

        Process** tmp = krealloc(g_procs, prevsize + stepsize);
        if (!tmp)
            return -ENOMEM;

        g_procs = tmp;
        memset((void*)g_procs + prevsize, 0, stepsize);
        g_proc_size += ALLOC_STEP;
    }
#undef ALLOC_STEP

    ++g_proc_count;
    g_procs[pid_idx] = proc;
    return 0;
}

static int procm_kill(Process* proc)
{
    /* FIXME: We don't shrink the g_procs array at all, we may want to cut
     * down on it sometimes if it gets too big and/or the system is starved for
     * memory ? */
    g_procs[proc->pid - 1] = NULL;
    g_last_free_proc_idx = proc->pid - 1;
    --g_proc_count;
    proc_free(proc);
    kfree(proc);
    return 0;
}

static int procm_try_kill_proc(Process* targetproc, Process* actingproc)
{
    if (actingproc == targetproc)
    {
        /* pid 1 has exited. */
        if (g_proc_count == 1)
            panic("PID 1 returned %d.", targetproc->exit_status);

        return -EINVAL;
    }

    return procm_kill(targetproc);
}

_INIT int procm_init()
{
    /* Select a scheduler */
    if (!g_scheduler_count)
        panic("No scheduler driver was registered!");

    Scheduler* sched = g_schedulers[0];
    for (size_t i = 0; i < g_scheduler_count; ++i)
    {
        Scheduler* tmp = g_schedulers[i];
        if (tmp->priority > sched->priority)
            sched = tmp;
    }

    g_active_sched = sched;
    KLOGF(
        INFO,
        "Using scheduler \"%s\" with priority %u.",
        g_active_sched->name,
        g_active_sched->priority);

    g_active_sched->procs = &g_procs;
    g_active_sched->proc_count = &g_proc_size;

    /* Reserve space for pid 1 */
    g_procs = kcalloc(1 * sizeof(Process*));
    if (!g_procs)
        panic("Failed to allocate space for pid 1!");

    g_proc_size = 1;
    g_last_free_proc_idx = 0;
    return 0;
}

pid_t procm_spawn_proc(
    const char* _USERPTR path,
    const char** argv,
    const char** envp,
    Process* actingproc)
{
    (void)argv;
    (void)envp;

    Process* newproc = kcalloc(sizeof(Process));
    if (!newproc)
        return -ENOMEM;

    int st = proc_init(newproc);
    if (st < 0)
    {
        kfree(newproc);
        return st;
    }

    st = proc_create_address_space(path, actingproc, newproc);
    if (st < 0)
    {
        kfree(newproc);
        return st;
    }

    st = proc_create_kernel_stack(newproc);
    if (st < 0)
    {
        proc_free(newproc);
        kfree(newproc);
        return -ENOMEM;
    }

    /* Change path */
    newproc->path = strdup(path);
    if (!newproc->path)
    {
        proc_free(newproc);
        kfree(newproc);
        return -ENOMEM;
    }

    newproc->pid = procm_available_pid();
    if (newproc->pid <= 0)
    {
        proc_free(newproc);
        kfree(newproc);
        return -ENOSPC;
    }

    st = procm_add_proc_to_pool(newproc);
    if (st < 0)
    {
        proc_free(newproc);
        kfree(newproc);
        return st;
    }

    return newproc->pid;
}

_INIT pid_t procm_spawn_kernel_proc()
{
    if (proc_init(&g_kernel_proc) < 0)
        panic("Failed to init kernel proc!");

    g_kernel_proc.paging_struct = mm_get_kernel_paging_struct();
    return 0;
}

_INIT pid_t procm_spawn_init()
{
    const char* initpath = "/bin/main";
    const char* argv[] = {NULL};
    const char* envv[] = {NULL};
    if (procm_spawn_proc(initpath, argv, envv, procm_get_kernel_proc()) != 1)
        panic("Failed to spawn init, tried \"%s\".", initpath);

    return 1;
}

int procm_mark_dead(int st, Process* proc)
{
    proc->exit_status = st;
    proc->zombie = true;
    return 0;
}

Process* procm_get_kernel_proc()
{
    return &g_kernel_proc;
}

int procm_sched_register(Scheduler* sched)
{
    Scheduler** tmp =
        krealloc(g_schedulers, (g_scheduler_count + 1) * sizeof(Scheduler*));
    if (!tmp)
        return -ENOMEM;

    g_schedulers = tmp;
    ++g_scheduler_count;
    g_schedulers[g_scheduler_count - 1] = sched;
    return 0;
}

int procm_sched_unregister(Scheduler* sched)
{
    Scheduler** found = NULL;
    for (size_t i = 0; i < g_scheduler_count; ++i)
    {
        if (g_schedulers[i] == sched)
        {
            found = &g_schedulers[i];
            break;
        }
    }

    if (!found)
        return -ENOENT;

    for (Scheduler** s = found; s < g_schedulers + g_scheduler_count - 1; ++s)
        *s = *(s + 1);

    --g_scheduler_count;
    return 0;
}

void procm_sched_start()
{
    if (!g_proc_size)
        panic("No pid 1 found to run!");

    if (g_proc_size > 1)
        panic("More than 1 process found, expected only pid 1!");

    if (g_active_sched->reset(g_active_sched) < 0)
        panic("Failed to preapre scheduler!");

    Process* pid1 = g_active_sched->current_proc(g_active_sched);
    proc_enter_initial(pid1);
}

Process* procm_sched_current_proc()
{
    return g_active_sched->current_proc(g_active_sched);
}

void procm_sched_yield()
{
    Process* current_proc = g_active_sched->current_proc(g_active_sched);
    current_proc->state = PROC_YIELDED;

    Process* next;
    while ((next = g_active_sched->next_proc(g_active_sched))->zombie)
        procm_try_kill_proc(next, current_proc);

    proc_switch(current_proc, next);
}

void sys_exit(int status)
{
    /* We can't straight up kill and free the curernt process, since that  would
     * mean freeing the process' kernel stack, which is also the stack we are
     * using right now. We deal with this by marking it as a zombie, and letting
     * the scheduling code clean it up when it knows it safe to do so. */
    procm_mark_dead(status, procm_sched_current_proc());
    procm_sched_yield();
    panic("procm_sched_yield() returned execution in sys_exit()!");
}
