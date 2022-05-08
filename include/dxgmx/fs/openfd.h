/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_OPENFD_H
#define _DXGMX_FS_OPENFD_H

#include <dxgmx/fs/vnode.h>
#include <posix/sys/types.h>

typedef struct S_OpenFileDescriptor
{
    /* The PID of the process that opened the file. */
    pid_t pid;
    /* The file descriptor number. */
    int fd;
    /* Access mode. */
    mode_t mode;
    /* Offset into the file */
    loff_t off;
    /* Pointer to the underlying vnode. */
    VirtualNode* vnode;
} OpenFileDescriptor;

#endif // !_DXGMX_FS_OPENFD_H
