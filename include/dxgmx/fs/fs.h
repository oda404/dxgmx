
#ifndef _DXGMX_FS_FS_H
#define _DXGMX_FS_FS_H

#include <dxgmx/fs/vnode.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>
#include <posix/sys/types.h>

struct S_FileSystem;
struct S_VirtualNode;

typedef struct S_FileSystemBacking
{
    union
    {
        const BlockDevice* blkdev;
    };
} FileSystemBacking;

/* A set of operations to act upon a FileSystem struct.
This defines the actual filesystem implementation. */
typedef struct S_FileSystemOperations
{
    char* name;
    /* Checks whether the given fs is valid. */
    int (*valid)(const BlockDevice*);
    /* Initializes the fs. */
    int (*init)(struct S_FileSystem*);
    /* Destroys the fs. */
    void (*destroy)(struct S_FileSystem*);

    ssize_t (*read)(
        struct S_FileSystem* fs,
        const struct S_VirtualNode* vnode,
        void* buf,
        size_t n,
        loff_t off);
} FileSystemOperations;

/* Represents a mounted/mountable filesystem. It's imeplementation resides
in 'operations'. */
typedef struct S_FileSystem
{
    /* The name of the mount. */
    char* mountsrc;
    /* Where this mountpoint resides */
    char* mountpoint;
    /* Mount flags */
    u32 mountflags;
    /* The filesystem implementation. */
    FileSystemOperations* operations;
    /* This is for use by the filesystem implementation. */
    void* operations_ctx;
    /* Vnodes cache for this filesystem. */
    struct S_VirtualNode* vnodes;
    size_t vnode_count;
    /* The thing backing this filesystem (disk/ram) */
    FileSystemBacking backing;
} FileSystem;

struct S_VirtualNode* fs_new_vnode(FileSystem* fs);

#endif // !_DXGMX_FS_FS_H
