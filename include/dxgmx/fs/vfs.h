/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_VFS_H
#define _DXGMX_FS_VFS_H

#include <dxgmx/fs/fs.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/types.h>
#include <posix/sys/types.h>

int vfs_init();

/** Mount a partition.
 * 'src' is the mount source, could be anything from a block device path, uuid,
 * or nothing at all. Only the respective driver knows what to do with this.
 * 'mntpoint' is the mount mount starting from the filesystem root.
 * 'type' is the name of the filesystem (driver). If null all filesystem drivers
 * are probed to see if any fit.
 * 'args' is a string of driver-specific options.
 * 'flags' are driver agnostic mount options.
 */
int vfs_mount(
    const char* src,
    const char* mntpoint,
    const char* type,
    const char* args,
    u32 flags);

int vfs_unmount(const char* src_or_mntpoint);

int vfs_register_fs_driver(FileSystemDriver);
int vfs_unregister_fs_driver(const char* name);

int vfs_open(const char* name, int flags, mode_t mode);

ssize_t vfs_read(int fd, void* buf, size_t n);

ssize_t vfs_write(int fd, const void* buf, size_t n);

off_t vfs_lseek(int fd, off_t offset, int whence);

int vfs_close(int fd);

#endif // !_DXGMX_FS_VFS_H
