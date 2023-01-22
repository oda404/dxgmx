
#ifndef _DXGMX_PROC_PROC_H
#define _DXGMX_PROC_PROC_H

#include <dxgmx/fs/openfd.h>
#include <dxgmx/mem/mm.h>
#include <posix/sys/types.h>

/* Structure representing a process. */
typedef struct S_Process
{
    /* Path to this binary. */
    char* path;

    /* pid of this process */
    pid_t pid;

    /* The paging structure used by this process. */
    PagingStruct paging_struct;

    /* List of open file descriptors. */
    OpenFileDescriptor* openfds;
    size_t openfd_count;

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
    bool dead;

    /* Return status of the process. */
    int exit_status;
} Process;

#endif // !_DXGMX_PROC_PROC_H
