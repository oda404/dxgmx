/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/elf/elfloader.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/string.h>
#include <dxgmx/task/task.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>

#define KLOGF_PREFIX "proc: "

static void proc_destroy_kernel_stack(Process* targetproc)
{
    if (!targetproc->kstack_top)
        return;

    kfree((void*)(targetproc->kstack_top - PROC_KSTACK_SIZE));
    targetproc->kstack_top = 0;
}

/* Create and map the stack for targetproc */
static int proc_create_stack(Process* targetproc)
{
    const ptr stack_top = PROC_HIGH_ADDRESS - PAGESIZE;
    const size_t stack_pages = PROC_STACK_SIZE / PAGESIZE;

    // FIXME: don't explictly map all stack pages here.
    for (size_t i = 0; i < stack_pages; ++i)
    {
        const ptr vaddr = stack_top - (i + 1) * PAGESIZE;
        int st = mm_new_user_page(vaddr, PAGE_RW, targetproc->paging_struct);
        if (st < 0)
        {
            KLOGF(
                ERR,
                "Failed to map a process' stack, (%zu/%zu) succeeded.",
                i,
                stack_pages);
            return st;
        }
    }

    targetproc->stack_top = stack_top;
    targetproc->stack_pagespan = stack_pages;
    targetproc->stack_ptr = targetproc->stack_top;
    /* This stack if freed whenever the process' paging struct is freed */
    return 0;
}

/* Copy the process from disk in memory. */
static int proc_load_from_file(
    const char* _USERPTR path, Process* actingproc, Process* targetproc)
{
    fd_t fd = vfs_open(path, O_RDONLY, 0, actingproc);
    if (fd < 0)
        return fd;

    int st = elfloader_load_from_file(fd, actingproc, targetproc);
    vfs_close(fd, actingproc);
    return st;
}

static int proc_enlarge_fds(Process* proc)
{
    /* Four ?? 4 ! */
#define STEP 4
    size_t prevsize = proc->fd_count * sizeof(bool);
    size_t stepsize = STEP * sizeof(bool);

    bool* tmp = krealloc(proc->fds, prevsize + stepsize);
    if (!tmp)
        return -ENOMEM;

    proc->fds = tmp;
    memset((void*)proc->fds + prevsize, 0, stepsize);
    proc->fd_count += STEP;
    return 0;
#undef STEP
}

int proc_init(Process* proc)
{
    memset(proc, 0, sizeof(Process));
    if (proc_enlarge_fds(proc) < 0)
        return -ENOMEM;

    proc->fd_last_free_idx = 0;
    proc->state = PROC_NEVER_RAN;
    /* FIXME: Hardcoded, this will break something in the future! */
    proc->dma_heap = (Heap){.vaddr = 2 * MIB, .pagespan = (2 * MIB) / PAGESIZE};
    return 0;
}

fd_t proc_new_fd(Process* proc)
{
    const size_t start_point = proc->fd_last_free_idx;
    while (proc->fd_last_free_idx < proc->fd_count &&
           proc->fds[proc->fd_last_free_idx])
    {
        ++proc->fd_last_free_idx;
        // overflow check, unlikey to happen but I don't want to bash my head in
        // if it does
        if (proc->fd_last_free_idx == start_point)
            return -EOVERFLOW;
    }

    if (proc->fd_last_free_idx == proc->fd_count)
    {
        if (proc_enlarge_fds(proc) < 0)
            return -ENOMEM;
    }

    proc->fds[proc->fd_last_free_idx] = true;
    return proc->fd_last_free_idx++;
}

void proc_free_fd(fd_t fd, Process* proc)
{
    if ((size_t)fd >= proc->fd_count)
        return; // Out of range

    proc->fds[fd] = false;
}

/* Create a new kernel stack for a process used for context switches */
int proc_create_kernel_stack(Process* targetproc)
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

void proc_free(Process* proc)
{
    if (proc->path)
    {
        kfree(proc->path);
        proc->path = NULL;
    }

    if (proc->paging_struct)
    {
        mm_destroy_paging_struct(proc->paging_struct);
        kfree(proc->paging_struct);
        proc->paging_struct = NULL;
    }

    if (proc->fds)
    {
        kfree(proc->fds);
        proc->fds = NULL;
        proc->fd_count = 0;
        proc->fd_last_free_idx = 0;
    }

    // FIXME: dma bitmap memleak :)

    proc_destroy_kernel_stack(proc);
}

int proc_create_address_space(
    const char* _USERPTR path, Process* actingproc, Process* targetproc)
{
    /* Initialize the paging structure */
    targetproc->paging_struct = kmalloc(sizeof(PagingStruct));
    if (!targetproc->paging_struct)
        return -ENOMEM;

    int st = mm_init_paging_struct(targetproc->paging_struct);
    if (st < 0)
        return st;

    /* Map the kernel */
    mm_map_kernel_into_paging_struct(targetproc->paging_struct);

    /* procm_load_from_file copies the binary from disk into memory */
    st = proc_load_from_file(path, actingproc, targetproc);
    if (st < 0)
        return st;

    st = proc_create_stack(targetproc);
    if (st < 0)
        return st;

    return 0;
}

void proc_enter_initial(Process* proc)
{
    /* Note to self: We may want to disable future preemption from happening
     * during all this */
    proc->state = PROC_RUNNING;
    mm_load_paging_struct(proc->paging_struct);
    task_set_impending_stack_top(proc->kstack_top);
    user_enter_arch(proc->inst_ptr, proc->stack_top);
}

void proc_switch(Process* curproc, Process* nextproc)
{
    ASSERT(nextproc->state != PROC_RUNNING);
    if (nextproc->state == PROC_NEVER_RAN)
    {
        proc_enter_initial(nextproc); // never returns
        ASSERT_NOT_HIT();
    }

    mm_load_paging_struct(nextproc->paging_struct);
    task_set_impending_stack_top(nextproc->kstack_top);
    task_switch(&curproc->task_ctx, &nextproc->task_ctx);
}
