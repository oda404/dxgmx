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
static size_t g_running_pids = 1;

static pid_t procm_next_available_pid()
{
    // FIXME: actually handle such cases
    if (g_running_pids >= PID_MAX)
        return -1;

    return g_running_pids++;
}

/**
 * Load a process' context. If successful, should be followed by a
 * userspace_jump2user.
 *
 * Returns:
 * 0 on sucess.
 * -errno values on error.
 */
static int procm_load_ctx(const Process* proc)
{
    extern int procm_arch_load_ctx(const Process* proc);

    /* Switch to this process' paging struct */
    int st = mm_load_paging_struct(&proc->paging_struct);
    if (st < 0)
        return st;

    /* Let the architecture do anything it needs */
    st = procm_arch_load_ctx(proc);
    return st;
}

/* Create a new kernel stack for a process used for context switches */
static int procm_create_proc_kstack(Process* targetproc)
{
    /* FIXME: I don't think this is a good ideea for many reasons, but it will
     * do for now. */
    ptr stack_top = (ptr)kmalloc_aligned(PROC_KSTACK_SIZE, sizeof(ptr));
    if (!stack_top)
        return -ENOMEM;

    /* The stack grows down, so we shift it's starting point. */
    stack_top += PROC_KSTACK_SIZE;
    targetproc->kstack_top = stack_top;
    return 0;
}

static void procm_destroy_proc_kstack(Process* targetproc)
{
    if (targetproc->kstack_top)
        kfree((void*)(targetproc->kstack_top - PROC_KSTACK_SIZE));
}

/* Create and map the stack for targetproc */
static int procm_create_proc_stack(Process* targetproc)
{
    ptr stage3_stack_top = PROC_HIGH_ADDRESS - PAGESIZE;

    // FIXME: don't explictly map all stack pages here.
    for (size_t i = 0; i < PROC_STACK_PAGESPAN; ++i)
    {
        ptr vaddr = stage3_stack_top - ((i + 1) * PAGESIZE);
        int st = mm_new_user_page(
            vaddr, PAGE_R | PAGE_W, &targetproc->paging_struct);

        if (st < 0)
            return st;
    }

    targetproc->stack_top = stage3_stack_top;
    targetproc->stack_pagespan = PROC_STACK_PAGESPAN;
    targetproc->stack_ptr = targetproc->stack_top;
    /* This stack if freed whenever the process' paging struct is freed */
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

static void procm_free_proc(Process* proc)
{
    if (proc->path)
        kfree(proc->path);

    mm_destroy_paging_struct(&proc->paging_struct);
    /* Make sure we don't pull the stack from under our feet. This can happen
     * when replacing a process. Let's hope this hack-ish solution doesn't
     * trigger some edge case... */
    if (proc->kstack_top != procm_arch_get_kstack_top())
        procm_destroy_proc_kstack(proc);
}

/* Copy the process from disk in memory. */
static int procm_load_proc(
    const char* _USERPTR path, Process* actingproc, Process* targetproc)
{
    fd_t fd = vfs_open(path, O_RDONLY, 0, actingproc);
    if (fd < 0)
        return fd;

    int st = elfloader_validate_file(fd, actingproc);
    if (st < 0)
        return st;

    st = elfloader_load_from_file(fd, actingproc, targetproc);

    vfs_close(fd, actingproc);

    return st;
}

static int procm_setup_proc_memory(
    const char* _USERPTR path, Process* actingproc, Process* targetproc)
{
    /* Initialize the paging structure */
    int st = mm_init_paging_struct(&targetproc->paging_struct);
    if (st < 0)
    {
        kfree(targetproc->path);
        return st;
    }

    /* Copy the kernel */
    mm_map_kernel_into_paging_struct(&targetproc->paging_struct);

    /* Once we swap paging structures all arguments marked _USERPTR will become
     * inaccessible, because their mapping is gone, so we make some copies while
     * we have the chance. */

    /* procm_load_proc copies the binary from disk into memory */
    st = procm_load_proc(path, actingproc, targetproc);
    if (st < 0)
        goto err;

    st = procm_create_proc_stack(targetproc);
    if (st < 0)
        goto err;

    return 0;

err:
    procm_free_proc(targetproc);
    return st;
}

static int procm_kill(Process* proc)
{
    procm_free_proc(proc);

    for (Process* p = proc; p < g_procs + g_proc_count - 1; ++p)
        *p = *(p + 1);

    --g_proc_count;
    /* FIXME: we have a memory leak here since the process array is not
     shrinked, nor is this empty space accounted for when adding new processes
     to the pool. */
    return 0;
}

int procm_replace_proc(
    const char* path, const char** argv, const char** envp, Process* actingproc)
{
    (void)argv;
    (void)envp;

    if (!path)
        return -EINVAL;

    /* When we replace a process, we keep everything from the old process except
     * it's path, and paging structure. Critically it's kernel stack is kept the
     * same. */

    Process newproc = {0};

    int st = procm_setup_proc_memory(path, actingproc, &newproc);
    if (st < 0)
        return st;

    /* Change path */
    newproc.path = strdup(path);
    if (!newproc.path)
    {
        procm_free_proc(&newproc);
        return -ENOMEM;
    }

    newproc.pid = actingproc->pid;
    newproc.fds = actingproc->fds;
    newproc.fd_count = actingproc->fd_count;
    newproc.kstack_top = actingproc->kstack_top;

    /* Load the context of the new process, so the old can be freed without any
     * worries. */
    procm_load_ctx(&newproc);
    procm_free_proc(actingproc);
    *actingproc = newproc;

    /* At this point the old process is gone, and the new one is waiting to
     * run */
    sched_yield();
}

pid_t procm_spawn_proc(
    const char* path, const char** argv, const char** envp, Process* actingproc)
{
    (void)argv;
    (void)envp;

    if (!path)
        return -EINVAL;

    Process newproc = {0};

    int st = procm_setup_proc_memory(path, actingproc, &newproc);
    if (st < 0)
        return st;

    st = procm_create_proc_kstack(&newproc);
    if (st < 0)
    {
        procm_free_proc(&newproc);
        return -ENOMEM;
    }

    /* Change path */
    newproc.path = strdup(path);
    if (!newproc.path)
    {
        procm_free_proc(&newproc);
        return -ENOMEM;
    }

    newproc.pid = procm_next_available_pid();
    if (newproc.pid <= 0)
    {
        procm_free_proc(&newproc);

        return -ENOSPC;
    }

    st = procm_add_proc_to_pool(&newproc);
    if (st < 0)
    {
        procm_free_proc(&newproc);
        return st;
    }

    return newproc.pid;
}

_INIT pid_t procm_spawn_kernel_proc()
{
    // FIXME: Hacky but works for now
    g_kernel_proc.paging_struct.data = mm_get_kernel_paging_struct()->data;
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

    procm_load_ctx(proc);

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
