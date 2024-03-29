/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_VFS_H
#define _DXGMX_FS_VFS_H

#include <dxgmx/fs/fs.h>
#include <dxgmx/fs/vnode.h>
#include <dxgmx/posix/fcntl.h>
#include <dxgmx/posix/sys/stat.h>
#include <dxgmx/posix/sys/types.h>
#include <dxgmx/posix/unistd.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/types.h>

int vfs_init();

/** Mount a partition.
 * 'src' is the mount source, could be anything from a block device path,
 * uuid, or nothing at all. Only the respective driver knows what to do with
 * this.
 *
 * 'mntpoint' is the mount point in the vfs.
 *
 * 'type' is the name of the filesystem (driver). If null all filesystem
 * drivers are probed to see if any fit.
 *
 * 'args' is a string of driver-specific options.
 *
 * 'flags' are driver agnostic mount options.
 *
 * Returns:
 * 0 on sucess
 * -EINVAL on invalid arguments.
 * -ENOMEM on out of memory.
 * other errnos come from the driver.
 */
int vfs_mount(
    const char* src,
    const char* mntpoint,
    const char* type,
    const char* args,
    u32 flags);

/**
 * Unmount a mounted filesystem.
 * 'src_or_dest' is the source or the mount point of a filesystem.
 *
 * Returns:
 * 0 on success
 * -EINVAL on invalid arguments
 * -ENOENT if no mounted filesystem was found by 'src_or_dest'.
 * other errnos come from the driver.
 */
int vfs_unmount(const char* src_or_dest);

int vfs_register_fs_driver(const FileSystemDriver* driver);

int vfs_unregister_fs_driver(const FileSystemDriver* driver);

/* File operations */

int vfs_rmfile(const char* path, Process* proc);

/** Open a file on behalf of a process
 * 'path' Path to the file.
 * 'flags' Open flags.
 * 'mode' If O_CREAT is specified in 'flags', then 'mode' is the create mode.
 * 'proc': The process.
 */
int vfs_open(const char* path, int flags, mode_t mode, Process* proc);

/**
 * Read from an opened file on behalf of a process.
 * 'fd' The file descriptor returned by vfs_open
 * 'buf' User buffer into which to read.
 * 'n' How many bytes to read.
 * 'proc' Acting process.
 */
ssize_t vfs_read(fd_t fd, void* buf, size_t n, Process* proc);

int vfs_ioctl(fd_t fd, int req, void* n, Process* proc);

/**
 * Write to an opened file on behalf of a process.
 * 'fd' The file descriptor returned by vfs_open
 * 'buf' User buffer to which to write.
 * 'n' How many bytes to write.
 * 'proc' Acting process.
 */
ssize_t vfs_write(fd_t fd, const void* buf, size_t n, Process* proc);

/**
 * Seek to a diferrent location in an opened file on behalf of a process.
 * 'fd' The file descriptor returned by vfs_open
 * 'offset' Offset in bytes.
 * 'whence' From where to seek. See SEEK_(SET/CUR/END).
 * 'proc' Acting process.
 */
off_t vfs_lseek(fd_t fd, off_t offset, int whence, Process* proc);

/**
 * Close an opened file on behalf of a process.
 * 'fd' The file descriptor returned by vfs_open.
 * 'proc' Acting process.
 */
int vfs_close(fd_t fd, Process* proc);

#endif // !_DXGMX_FS_VFS_H
