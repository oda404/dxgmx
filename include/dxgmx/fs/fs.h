/**
 * Copyright 2022 Alexandru Olaru.
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

#define FILESYSTEM_BACKING_DISK 1
#define FILESYSTEM_BACKING_RAM 2

/* Defines the operations of a filesystem driver implementation. */
typedef struct S_FileSystemDriver
{
    char* name;
    u8 backing;

    /* Checks whether the given BlockDevice holds a valid filesystem. Chould be
     * left NULL, if the filesystem is backed by ram. */
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

    ssize_t (*write)(
        struct S_FileSystem* fs,
        struct S_VirtualNode* vnode,
        const void* buf,
        size_t n,
        loff_t off);

    /* Create a new empty file on the filesystem. */
    int (*mkfile)(struct S_FileSystem* fs, const char* path, mode_t mode);

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

struct S_VirtualNode*
fs_new_vnode(FileSystem* fs, ino_t n, const VirtualNode* parent);
int fs_rm_vnode(FileSystem* fs, struct S_VirtualNode* vnode);

#endif // !_DXGMX_FS_FS_H
