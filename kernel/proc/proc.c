/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/elf/elfloader.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/proc_limits.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>

#define KLOGF_PREFIX "proc: "

extern int proc_arch_load_ctx(const Process* proc);
extern ptr proc_arch_get_current_kstack_top();

static void proc_destroy_kernel_stack(Process* targetproc)
{
    if (!targetproc->kstack_top)
    {
        KLOGF(
            ERR,
            "Tried to free non-existent kstack for pid %zu.",
            targetproc->pid);
        return;
    }

    kfree((void*)(targetproc->kstack_top - PROC_KSTACK_SIZE));
    targetproc->kstack_top = 0;
}

/* Create and map the stack for targetproc */
static int proc_create_stack(Process* targetproc)
{
    const ptr stack_top = PROC_HIGH_ADDRESS - PAGESIZE;
    const size_t stack_pages = PROC_STACK_PAGESPAN;

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
    memset(proc->fds + prevsize, 0, stepsize);
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

    if (proc->fd_last_free_idx >= proc->fd_count)
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

/**
 * Load a process' context. If successful, should be followed by a
 * userspace_jump2user.
 *
 * Returns:
 * 0 on sucess.
 * -errno values on error.
 */
int proc_load_ctx(const Process* proc)
{
    /* Switch to this process' paging struct */
    int st = mm_load_paging_struct(proc->paging_struct);
    if (st < 0)
        return st;

    /* Let the architecture do anything it needs */
    return proc_arch_load_ctx(proc);
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

    mm_destroy_paging_struct(proc->paging_struct);
    kfree(proc->paging_struct);
    proc->paging_struct = NULL;

    /* Make sure we don't pull the stack from under our feet. This can
     * happen when replacing a process. Let's hope this hack-ish solution
     * doesn't trigger some edge case... */
    if (proc->kstack_top != proc_arch_get_current_kstack_top())
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
    {
        proc_free(targetproc);
        return st;
    }

    /* Map the kernel */
    mm_map_kernel_into_paging_struct(targetproc->paging_struct);

    /* procm_load_from_file copies the binary from disk into memory */
    st = proc_load_from_file(path, actingproc, targetproc);
    if (st < 0)
    {
        proc_free(targetproc);
        return st;
    }

    st = proc_create_stack(targetproc);
    if (st < 0)
    {
        proc_free(targetproc);
        return st;
    }

    return 0;
}
