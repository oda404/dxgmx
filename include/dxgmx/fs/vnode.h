/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_VNODE_H
#define _DXGMX_FS_VNODE_H

#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>
#include <posix/sys/types.h>

struct S_FileSystem;

/* If the file has been removed but metadata bits are still
present on disk. */
#define INODE_STATE_WILL_FREE 1

typedef struct S_VirtualNode
{
    /* The name of the file. */
    char* name;
    /* Arbitrarily unique number for the 'owner' filesystem. May
    or may not mean something to the underlying filesystem. */
    ino_t n;
    /* The size of the vnode in bytes. */
    size_t size;
    /* Access mode. */
    mode_t mode;
    /* If the file has been removed but is not  */
    u16 state;
    /* The parent directory's inode number. This is 0 for /. */
    ino_t parent_n;
    /* The filesystem backing this vnode. */
    struct S_FileSystem* owner;

    union
    {
        /* If (mode & S_IFMT) == S_IFBLK */
        const BlockDevice* blkdev;
    } private;
} VirtualNode;

#endif // !_DXGMX_FS_VNODE_H
