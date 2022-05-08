/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_VFS_H
#define _DXGMX_FS_VFS_H

#include <dxgmx/fs/fs.h>
#include <dxgmx/types.h>

bool vfs_init();

int vfs_mount(const char* src, const char* dest, u32 flags);
int vfs_unmount(const char* src_or_dest);
int vfs_mount_by_uuid(const char* uuid, const char* dest, u32 flags);

int vfs_register_fs_driver(const FileSystemDriver*);
int vfs_unregister_fs_driver(const char* name);

#endif // !_DXGMX_FS_VFS_H
