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

#define KLOGF_PREFIX "procmanager: "

static Process* g_procs = NULL;
static size_t g_proc_count = 0;
static size_t g_next_queued_proc_idx = 0;
static size_t g_running_pids = 1;

static pid_t procm_next_available_pid()
{
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
static int procm_load_ctx(Process* proc)
{
    extern int procm_arch_load_ctx(Process * proc);

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
    ptr stage3_stack_top = PROC_VIRTUAL_HIGH_ADDRESS - PAGESIZE;

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

static void procm_free_proc_data(Process* proc)
{
    if (proc->path)
        kfree(proc->path);

    procm_destroy_proc_kstack(proc);

    mm_destroy_paging_struct(&proc->paging_struct);
}

static void procm_free_proc(Process* proc)
{
    // FIXME: shrink array
    procm_free_proc_data(proc);
    kfree(proc);
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

    st = procm_create_proc_kstack(targetproc);
    if (st < 0)
        goto err;

    return 0;

err:
    procm_free_proc_data(targetproc);
    return st;
}

int procm_replace_proc(
    const char* path, const char** argv, const char** envp, Process* actingproc)
{
    (void)argv;
    (void)envp;

    if (!path)
        return -EINVAL;

    /* Make a copy of the old process. procm_setup_proc_memory will make the
     * needed changes. */
    Process newproc = {0};

    int st = procm_setup_proc_memory(path, actingproc, &newproc);
    if (st < 0)
        return st;

    /* Change path */
    newproc.path = strdup(path);
    if (!newproc.path)
    {
        procm_free_proc_data(&newproc);
        return -ENOMEM;
    }

    newproc.pid = actingproc->pid;
    newproc.fds = actingproc->fds;
    newproc.fd_count = actingproc->fd_count;

    procm_free_proc_data(actingproc);

    *actingproc = newproc;

    /* At this point the old process is gone, and the new one is waiting to
     * run
     */
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

    /* Change path */
    newproc.path = strdup(path);
    if (!newproc.path)
    {
        procm_free_proc_data(&newproc);
        return -ENOMEM;
    }

    newproc.pid = procm_next_available_pid();
    if (newproc.pid <= 0)
    {
        procm_free_proc_data(&newproc);
        return -ENOSPC;
    }

    st = procm_add_proc_to_pool(&newproc);
    if (st < 0)
    {
        procm_free_proc_data(&newproc);
        return st;
    }

    return newproc.pid;
}

pid_t procm_spawn_init()
{
    Process proc = {0};

    proc.pid = procm_next_available_pid();
    ASSERT(proc.pid == 1);

    int st = mm_init_paging_struct(&proc.paging_struct);
    if (st < 0)
        return st;

    st = mm_map_kernel_into_paging_struct(&proc.paging_struct);
    if (st < 0)
        goto err;

    /* Model the binary image in memory */
    const size_t stage3_text_size =
        kimg_kinit_stage3_text_end() - kimg_kinit_stage3_text_start();

    ASSERT(stage3_text_size);

    const size_t stage3_text_size_aligned =
        bytes_align_up64(stage3_text_size, PAGESIZE);

    /* Text section, we copy the .kinit_stage3_text section into the new
     * process' text. */
    for (size_t page = 0; page < stage3_text_size_aligned; page += PAGESIZE)
    {
        ptr vaddr = PROC_VIRTUAL_START_OFFSET + (page * PAGESIZE);

        /* I'm not event going to bother clearing PAGE_W as we spend so little
         * time in kinit_stage3 and it's a *known* environment */
        st = mm_new_user_page(
            vaddr, PAGE_R | PAGE_W | PAGE_X, &proc.paging_struct);

        if (st < 0)
            goto err;

        /* FIXME: Enforce page permissions once we have the API for that. */
    }

    /* Creat process' stack */
    st = procm_create_proc_stack(&proc);
    if (st)
        goto err;

    /* Create the process' kernel stack. */
    st = procm_create_proc_kstack(&proc);
    if (st < 0)
        goto err;

    /* Switch to the process' paging struct, and start copying stuff. */
    mm_load_paging_struct(&proc.paging_struct);

    /* Copy entire text section, meaning one function */
    memcpy(
        (void*)PROC_VIRTUAL_START_OFFSET,
        (void*)kimg_kinit_stage3_text_start(),
        stage3_text_size);

    /* Go back to the kernel's paging struct. FIXME: Really necessary ? */
    mm_load_kernel_paging_struct();

    /* Since the kinit_stage3 sections contains one thing (a function), we can
     * just point it to the beginning of where that section was coppied. */
    proc.inst_ptr = PROC_VIRTUAL_START_OFFSET;

    st = procm_add_proc_to_pool(&proc);
    if (st < 0)
        goto err;

    return proc.pid;

err:
    procm_free_proc_data(&proc);
    return st;
}

int procm_kill(Process* proc)
{
    if (!proc)
        return -ENOENT;

    procm_free_proc(proc);
    return 0;
}

int procm_mark_dead(int st, Process* proc)
{
    proc->exit_status = st;
    proc->dead = true;
    return 0;
}

void procm_switch_ctx(Process* proc)
{
    ASSERT(proc);

    procm_load_ctx(proc);

    user_jump2user(proc->inst_ptr, proc->stack_ptr);
}

size_t procm_proc_count()
{
    return g_proc_count;
}

int procm_try_kill_proc(Process* actingproc, Process* targetproc)
{
    if (actingproc == targetproc)
    {
        /* pid 1 has exited. */
        if (g_proc_count == 1)
            panic("PID 1 returned %d, halt.", targetproc->exit_status);

        return -EINVAL;
    }

    procm_kill(targetproc);
    return 0;
}

Process* procm_next_queued_proc()
{
    if (g_next_queued_proc_idx >= g_proc_count)
        g_next_queued_proc_idx = 0;

    return &g_procs[g_next_queued_proc_idx++];
}
