/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/elf/elf.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/userspace.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "procmanager: "

/* This is the bottom of the address space for a process. */
#define PROC_VIRTUAL_START_OFFSET (128UL * MIB)

/* This is the top of the address space for a process, beyond here (3-4GIB) lays
 * the kernel mapping, which is not directly accessible to a user process. */
#define PROC_VIRTUAL_HIGH_ADDRESS (3UL * GIB)

/* Size of a process' kernel stack. */
#define PROC_KSTACK_SIZE (PAGESIZE)

/* Default stack size for a process. */
#define PROC_STACK_SIZE (8 * KIB)

static Process* g_procs = NULL;
static size_t g_proc_count = 0;
static size_t g_running_pids = 1;

/* Architecture specific function for loading a process' ctx. */
int procm_arch_load_ctx(Process* proc);

/* Create a new kernel stack for a process used for context switches */
static ptr procm_new_proc_kstack()
{
    /* I don't think this is a good ideea for many reasons, but it will do for
     * now. */
    ptr stack_top = (ptr)kmalloc_aligned(PROC_KSTACK_SIZE, sizeof(ptr));
    if (!stack_top)
        return ENOMEM;

    /* stack grows down */
    stack_top += PROC_KSTACK_SIZE;

    return stack_top;
}

static void procm_free_proc_kstack(ptr kstack_top)
{
    kstack_top -= PROC_KSTACK_SIZE;
    kfree((void*)kstack_top);
}

static Process* procm_new_proc()
{
    Process proc = {0};

    int st = mm_init_paging_struct(&proc.paging_struct);
    if (st < 0)
        return NULL;

    proc.pid = g_running_pids++;

    /* Allocate new Process */
    Process* tmp = krealloc(g_procs, sizeof(Process) * (g_proc_count + 1));
    if (!tmp)
    {
        mm_destroy_paging_struct(&proc.paging_struct);
        return NULL;
    }

    g_procs = tmp;
    ++g_proc_count;

    g_procs[g_proc_count - 1] = proc;
    return &g_procs[g_proc_count - 1];
}

static void procm_free_proc(Process* proc)
{
    // FIXME: shrink array
    if (proc->path)
        kfree(proc->path);

    mm_destroy_paging_struct(&proc->paging_struct);
    procm_free_proc_kstack(proc->kstack_top);
    kfree(proc);
}

static int procm_create_proc_stack(size_t pagespan, Process* proc)
{
    ptr stage3_stack_top = PROC_VIRTUAL_HIGH_ADDRESS - PAGESIZE;

    // FIXME: don't explictly map all stack pages here.
    for (size_t i = 0; i < pagespan; ++i)
    {
        int st = mm_new_user_page(
            stage3_stack_top - ((i + 1) * PAGESIZE), 0, &proc->paging_struct);

        if (st)
            return st;
    }

    proc->stack_top = stage3_stack_top;
    proc->stack_pagespan = pagespan;
    proc->stack_ptr = proc->stack_top - sizeof(ptr);

    return 0;
}
pid_t procm_spawn_proc(const char* path)
{
    (void)path;
    TODO_FATAL();
}

pid_t procm_spawn_init()
{
    Process* proc = procm_new_proc();
    if (!proc)
        return -ENOMEM;

    int st = mm_map_kernel_into_paging_struct(&proc->paging_struct);
    if (st < 0)
    {
        KLOGF(ERR, "Could not map kernel into PID 1's paging struct.");
        return st;
    }

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
        mm_new_user_page(
            PROC_VIRTUAL_START_OFFSET + (page * PAGESIZE),
            0,
            &proc->paging_struct);

        /* FIXME: Enforce page permissions once we have the API for that. */
    }

    /* Creat process stack */
    st = procm_create_proc_stack(PROC_STACK_SIZE / PAGESIZE, proc);
    if (st)
        return st;

    /* Switch to the process' paging struct, and start copying stuff. */
    mm_load_paging_struct(&proc->paging_struct);

    /* Copy entire text section, meaning one function */
    memcpy(
        (void*)PROC_VIRTUAL_START_OFFSET,
        (void*)kimg_kinit_stage3_text_start(),
        stage3_text_size);

    /* Go back to the kernel's paging struct. */
    mm_load_kernel_paging_struct();

    /* Since the kinit_stage3 sections contains one thing (a function), we can
     * just point it to the beginning of where that section was coppied. */
    proc->inst_ptr = PROC_VIRTUAL_START_OFFSET;

    /* Create the process' kernel stack. */
    proc->kstack_top = procm_new_proc_kstack();
    if (proc->kstack_top < kimg_vaddr())
    {
        procm_free_proc(proc);
        /* It's an errno since the kernel stack is always after kimg_vaddr */
        return -proc->kstack_top;
    }

    return proc->pid;
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

int procm_load_ctx(Process* proc)
{
    ASSERT(proc);

    int st = mm_load_paging_struct(&proc->paging_struct);
    if (st < 0)
        return st;

    st = procm_arch_load_ctx(proc);

    return st;
}

void procm_switch_ctx(Process* proc)
{
    ASSERT(proc);

    if (procm_load_ctx(proc) < 0)
        return;

    userspace_jump2user(proc->inst_ptr, proc->stack_ptr);
}

Process* procm_proc_from_pid(pid_t pid)
{
    FOR_EACH_ELEM_IN_DARR (g_procs, g_proc_count, proc)
    {
        if (proc->pid == pid)
            return proc;
    }

    return NULL;
}

Process* procm_procs()
{
    return g_procs;
}

size_t procm_proc_count()
{
    return g_proc_count;
}
