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

bool vfs_init();

int vfs_mount(const char* src, const char* dest, u32 flags);
int vfs_unmount(const char* src_or_dest);
int vfs_mount_by_uuid(const char* uuid, const char* dest, u32 flags);
int vfs_mount_ramfs(const char* driver_name, const char* dest, u32 flags);

int vfs_register_fs_driver(const FileSystemDriver*);
int vfs_unregister_fs_driver(const char* name);

int vfs_open(const char* name, int flags, mode_t mode, pid_t pid);
ssize_t vfs_read(int fd, void* buf, size_t n, pid_t pid);
int vfs_close(int fd, pid_t pid);

#endif // !_DXGMX_FS_VFS_H
