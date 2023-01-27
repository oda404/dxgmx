/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_OPENFD_H
#define _DXGMX_FS_OPENFD_H

#include <dxgmx/fs/vnode.h>
#include <posix/sys/types.h>

/* Global file descriptor. A process only holds an int that it gives to the vfs
 * to find the global file descriptor I.E. this */
typedef struct S_FileDescriptor
{
    /* The file descriptor number. */
    int fd;

    /* The PID of the process that opened the file. */
    pid_t pid;

    /* open() flags. */
    int flags;

    /* Access mode. */
    mode_t mode;

    /* Read/write offset into the file */
    loff_t off;

    /* Pointer to the underlying vnode. */
    VirtualNode* vnode;
} FileDescriptor;

#endif // !_DXGMX_FS_OPENFD_H
