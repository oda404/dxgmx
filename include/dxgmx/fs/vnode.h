
#ifndef _DXGMX_FS_VNODE_H
#define _DXGMX_FS_VNODE_H

#include <dxgmx/types.h>
#include <posix/sys/types.h>

struct S_FileSystem;

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
    /* The parent directory for this vnode. This is NULL for /. */
    struct S_VirtualNode* parent;
    /* The virtual device backing this vnode. */
    struct FileSystem* owner;
} VirtualNode;

#endif // !_DXGMX_FS_VNODE_H
