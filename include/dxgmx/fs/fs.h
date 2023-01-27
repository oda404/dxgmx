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

struct S_FileSystemDriver;

/* Represents a mounted/mountable filesystem. It's imeplementation resides
in 'operations'. */
typedef struct S_FileSystem
{
    /* The name of the mount. */
    char* mntsrc;
    /* Where this mountpoint resides */
    char* mntpoint;
    /* Mount flags */
    u32 flags;
    /* The filesystem driver. */
    struct S_FileSystemDriver* driver;
    /* This is free for use by the filesystem driver to store whatever. Like the
     * block device (if the filesystem is backed by a block device) */
    void* driver_ctx;
    /* Vnodes cache for this filesystem. */
    VirtualNode* vnodes;
    size_t vnode_count;
} FileSystem;

/**
 * Defines the operations of a filesystem driver implementation.
 * Any arguments passed to any functions MAY be null, unless otherwise
 * specified. A filesystem driver module should register it's own
 * FileSystemDriver struct using vfs_register_fs_driver().
 */
typedef struct S_FileSystemDriver
{
    char* name;

    /* Checks whether the given BlockDevice holds a valid filesystem. Chould be
     * left NULL, if the filesystem is backed by ram. */
    int (*valid)(const BlockDevice*);

    /** Prepare 'fs' for mounting.
     * 'src' is the mount source
     *
     * 'type' is the driver name the filesystem is expecting. If the type does
     * not match the driver's name, the driver should instantly return with a
     * -EINVAL.
     *
     * 'args' string with driver specific arguments.
     *
     * 'fs' is the target filesystem. The filesystem will already have
     * 'mountpoint' and 'flags' set. Also 'driver' will be set to this driver.
     * The driver should not touch the before-mentioned fields. Guaranteed to be
     * non-null.
     *
     * This function is not explicitly passed the mount point, as the driver
     * should be agnostic to that.
     */
    int (*init)(
        const char* src, const char* type, const char* args, FileSystem* fs);

    /* Destroys the fs. */
    void (*destroy)(FileSystem*);

    ssize_t (*read)(
        FileSystem* fs,
        const struct S_VirtualNode* vnode,
        void* buf,
        size_t n,
        loff_t off);

    ssize_t (*write)(
        FileSystem* fs,
        struct S_VirtualNode* vnode,
        const void* buf,
        size_t n,
        loff_t off);

    /* Create a new empty file on the filesystem. */
    int (*mkfile)(FileSystem* fs, const char* path, mode_t mode);
    int (*mkdir)(FileSystem* fs, const char* path, mode_t mode);

} FileSystemDriver;

struct S_VirtualNode*
fs_new_vnode(FileSystem* fs, ino_t n, const VirtualNode* parent);
int fs_rm_vnode(FileSystem* fs, struct S_VirtualNode* vnode);
/* Takes in a given path and makes sure it is relative to the given filesystem.
 */
int fs_make_path_relative(const FileSystem* fs, char* path);
int fs_make_path_canonical(char* path);
VirtualNode* fs_vnode_for_path(FileSystem* fs, const char* path);

#endif // !_DXGMX_FS_FS_H
