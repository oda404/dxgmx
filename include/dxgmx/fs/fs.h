/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_FS_H
#define _DXGMX_FS_FS_H

#include <dxgmx/fs/vnode.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/linkedlist.h>

struct S_FileSystemDriver;

/* A mounted filesystem. */
typedef struct S_FileSystem
{
    /* The name of the mount. */
    const char* mntsrc;

    /* Where this mountpoint resides */
    const char* mntpoint;

    const char* args;

    /* Mount flags */
    u32 flags;

    /* The filesystem driver. */
    const struct S_FileSystemDriver* driver;

    /* This is free for use by the filesystem driver to store whatever. Like the
     * block device (if the filesystem is backed by a block device) */
    void* driver_ctx;

    /* Vnode cache for this filesystem. */
    LinkedList vnode_ll;
} FileSystem;

DEFINE_ERR_OR_PTR(FileSystem);

/**
 * Defines the operations of a filesystem driver implementation.
 * Any arguments passed to any functions MAY be null, unless otherwise
 * specified. A filesystem driver module should register it's own
 * FileSystemDriver struct using vfs_register_fs_driver().
 */
typedef struct S_FileSystemDriver
{
    /* Driver name */
    char* name;

    /* True if the driver can be probed for a specific source. Examples of this
     * include disk backed filesystems (fat, ext2). False for stuff like ram
     * backed filesystems. */
    bool generic_probe;

    /**
     * Check if the filesystem on 'src' is valid. If the filesystem is valid it
     * is initialized.
     *
     * 'src' Non-NULL string describing the source of the filesystem. May or may
     * not mean anything to the driver.
     * 'type' Filesystem type. If NULL the driver should validate this
     * filesystem. If 'type' holds a string, the driver will only try to
     * validate the filesystem, if strcmp(type, driver_name) == 0
     * 'args' String containing arguments for the filesystem.
     * 'fs' Non-NULL target filesystem.
     *
     * Returns:
     * 0 on sucess
     * negative errnos, indicating errors on failure.
     */
    int (*init)(FileSystem* fs);

    /**
     * Destroy a filesystem. Right before the filesystem is unmounted by the
     * vfs, this function is called, to clean up anything the driver allocated.
     *
     * 'fs' Non-NULL target filesystem.
     */
    void (*destroy)(FileSystem* fs);

    ERR_OR(ino_t)
    (*mkfile)(
        VirtualNode* dir,
        const char* name,
        mode_t mode,
        uid_t uid,
        gid_t gid,
        struct S_FileSystem* fs);

    int (*rmnode)(VirtualNode* vnode);

    /* Default operations that performed on this filesystem's vnodes. */
    const VirtualNodeOperations* vnode_ops;

} FileSystemDriver;

/**
 * Create a new vnode cache for 'fs'. The returned vnode has vnode->owner set to
 * 'fs', vnode->ops set to fs->vnode_ops, and the rest of the fields zeroed out.
 *
 * 'fs' Non null pointer to the target filesystem for which to create the vnode.
 *
 * Returns:
 * A VirtualNode* on sucess.
 * NULL on out of memory.
 */
VirtualNode* fs_new_vnode_cache(const char* name, FileSystem* fs);

/**
 * Remove a vnode from the vnode cache of 'fs'.
 *
 * No null pointers should be passed to this function.
 *
 * 'vnode' VirtualNode to be removed.
 * 'fs' The target filesyste.
 *
 * Returns:
 * 0 on sucess.
 * -ENOENT if 'vnode' is not in the vnode cache of 'fs'.
 */
int fs_free_cached_vnode(VirtualNode* vnode, FileSystem* fs);

/**
 * Free all cached vnodes of a filesystem.
 *
 * No null pointers should be passed to this function.
 *
 * 'fs' The target filesyste.
 *
 * Returns:
 * 0 on sucess.
 */
int fs_free_all_cached_vnodes(FileSystem* fs);

/**
 * Lookup the vnode for 'path' that is residing on 'fs'. This function first
 * tries calling fs_lookup_vnode_cached_relative. If that fails it invokes the
 * filesystem driver to do a lookup internally.
 *
 * No null pointers should be passed to this function.
 *
 * 'path' An absolute path.
 * 'fs' The filesystem to scan.
 *
 * Returns:
 * ERR_OR_PTR(VirtualNode)
 * value: A VirtualNode*
 * error:
 *  -ENOENT if no vnode was found
 */
ERR_OR_PTR(VirtualNode) fs_lookup_vnode(const char* path, FileSystem* fs);

VirtualNode* fs_ino_to_vnode(ino_t ino, FileSystem* fs);

ERR_OR(ino_t)
fs_mkfile(const char* path, mode_t mode, uid_t uid, gid_t gid, FileSystem* fs);

#endif // !_DXGMX_FS_FS_H
