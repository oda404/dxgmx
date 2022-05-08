/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_VNODE_H
#define _DXGMX_FS_VNODE_H

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
    /* The parent directory for this vnode. This is NULL for /. */
    struct S_VirtualNode* parent;
    /* The filesystem backing this vnode. */
    struct S_FileSystem* owner;
} VirtualNode;

#endif // !_DXGMX_FS_VNODE_H
