/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_VNODE_H
#define _DXGMX_FS_VNODE_H

#include <dxgmx/types.h>
#include <posix/sys/types.h>

struct S_FileSystem;
struct S_VirtualNodeOperations;

/* If the file has been removed but metadata bits are still
present on disk. */
#define INODE_STATE_WILL_FREE 1

/* Virtual node representing an object in the vfs, be it a file, dir, link,
 * whatever. Note that you should be careful when allocating and storing a bunch
 * of VirtualNodes for a filesystem, as VirtualNode*s are stored as
 * caches in things like a FileDescriptor, which means that you can't just have
 * a VirtualNode[] and call krealloc on it, as krealloc may move the memory
 * location of the block, leaving such caches invalid. The big VirtualNode*
 * cache in FileSystem is imlpemented using a linked list for this exact reason.
 * Maybe there is a better solution to this, but for now it works.
 */
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
    uid_t uid;
    gid_t gid;
    /* If the file has been removed but is not  */
    u16 state;

    /* The parent directory VirtualNode, NULL for / */
    const struct S_VirtualNode* parent;

    /* The filesystem backing this vnode. */
    struct S_FileSystem* owner;

    const struct S_VirtualNodeOperations* ops;
} VirtualNode;

/**
 * Operations performed on a vnode. Each member of this struct is a function
 * pointer that should be implemented by filesystem drivers. This struct
 * is used into two places:
 * - 1st: Each filesystem driver holds a pointer to this struct describing how
 * the filesystem should perform certaing operations on it's files.
 * - 2nd: Each VirtualNode of each FileSystem also holds a pointer to one of
 * these structs. If we are dealing with "normal" files, that pointer will be
 * the same as the driver's. But if we are dealing with special files for
 * example, block devices, that pointer will point to some other
 * VirtualNodeOperations specific for reading from a block device.
 *
 * For some reason this seems like an awkward solution, but it's the only decent
 * one I've come up with so far.
 */
typedef struct S_VirtualNodeOperations
{
    /**
     * Read from a file described by a vnode.
     *
     * No null pointers should be passed to this function.
     *
     * 'vnode' The target vnode.
     * 'buf' The destination buffer into which to read.
     * 'n' How many bytes to read.
     * 'off' Offset into the file.
     *
     * Returns:
     * Non negative integer describing how many bytes were read on success.
     */
    ssize_t (*read)(const VirtualNode* vnode, void* buf, size_t n, loff_t off);

    /**
     * Write to a file described by a vnode.
     *
     * No null pointers should be passed to this function.
     *
     * 'vnode' The target vnode.
     * 'buf' The src buffer from which to write.
     * 'n' How many bytes to write.
     * 'off' Offset into the file.
     *
     * Returns:
     * Non negative integer describing how many bytes were written on success.
     */
    ssize_t (*write)(VirtualNode* vnode, const void* buf, size_t n, loff_t off);

} VirtualNodeOperations;

#endif // !_DXGMX_FS_VNODE_H
