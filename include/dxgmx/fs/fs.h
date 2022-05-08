/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

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

/* Defines the operations of a filesystem driver implementation. */
typedef struct S_FileSystemDriver
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

    struct S_VirtualNode* (*vnode_for_path)(
        struct S_FileSystem* fs, const char* path);
} FileSystemDriver;

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
    /* The filesystem driver. */
    FileSystemDriver* driver;
    /* This is free for use by the filesystem driver to store whatever. */
    void* driver_ctx;
    /* Vnodes cache for this filesystem. */
    struct S_VirtualNode* vnodes;
    size_t vnode_count;
    /* The thing backing this filesystem (disk/ram) */
    FileSystemBacking backing;
} FileSystem;

struct S_VirtualNode* fs_new_vnode(FileSystem* fs);

#endif // !_DXGMX_FS_FS_H
