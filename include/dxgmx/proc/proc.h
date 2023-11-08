/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PROC_PROC_H
#define _DXGMX_PROC_PROC_H

#include <dxgmx/attrs.h>
#include <dxgmx/fs/fd.h>
#include <dxgmx/mem/heap.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/task/task.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bitmap.h>

typedef int fd_t;
DEFINE_ERR_OR(fd_t);

typedef enum ProcessState
{
    PROC_NEVER_RAN = 0x1,
    PROC_RUNNING = 0x2,
    PROC_YIELDED = 0x4,
    PROC_PREEMPTED = PROC_YIELDED | 0x8,
    PROC_BLOCKED = PROC_YIELDED | 0x10
} ProcessState;

/* Structure representing a process. */
typedef struct S_Process
{
    /* Path to this binary. */
    char* path;

    /* pid of this process */
    pid_t pid;

    /* The paging structure used by this process. */
    PagingStruct* paging_struct;

    Heap dma_heap;
    Bitmap dma_bitmap;

    /**
     * Lookup table of process file descriptors
     */
    bool* fds;
    size_t fd_count;
    size_t fd_last_free_idx;

    /* Instruction pointer to which we should jump when running this process */
    ptr inst_ptr;

    /* Start address of the stack. */
    ptr stack_top;

    /* Pagespan of the stack. */
    size_t stack_pagespan;

    /* Stack pointer we should load when running this process */
    ptr stack_ptr;

    /* Top of the kernel stack used for this process' context switches. */
    ptr kstack_top;

    /* Process has been terminated and is waiting to be freed up. */
    bool zombie;

    /* Return status of the process. */
    int exit_status;

    TaskContext task_ctx;

    ProcessState state;
} Process;

int proc_init(Process* proc);
void proc_free(Process* proc);

fd_t proc_new_fd(Process* proc);
void proc_free_fd(fd_t fd, Process* proc);

/* Create a new kernel stack for a process used for context switches */
int proc_create_kernel_stack(Process* targetproc);

int proc_create_address_space(
    const char* path, Process* actingproc, Process* targetproc);

_ATTR_NORETURN void proc_enter_initial(Process* proc);
void proc_switch(Process* curproc, Process* nextproc);

#endif // !_DXGMX_PROC_PROC_H
