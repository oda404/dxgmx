/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_FD_H
#define _DXGMX_FS_FD_H

#include <dxgmx/fs/vnode.h>
#include <posix/sys/types.h>

/* Global file descriptor. A process only holds an int that it gives to the vfs
 * to find the global file descriptor I.E. this */
typedef struct S_FileDescriptor
{
    /* The process file descriptor number. */
    int fd;

    /* The PID of the process that opened the file. This combined with 'fd'
     * makes this FileDescriptor unique. */
    pid_t pid;

    /* open() flags. */
    int flags;

    /* Read/write offset into the file */
    off_t off;

    /* Pointer to the underlying vnode. */
    VirtualNode* vnode;
} FileDescriptor;

#endif // !_DXGMX_FS_FD_H
