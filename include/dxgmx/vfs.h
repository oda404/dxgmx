/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_VFS_H
#define _DXGMX_VFS_H

#include <dxgmx/storage/drive.h>
#include <dxgmx/types.h>

bool vfs_add_drive(const GenericDrive* drive);
bool vfs_remove_drive();
bool vfs_mount(const char* name, const char* mountpoint);
bool vfs_umount(const char* name_or_mountpoint);
bool vfs_init();

#endif // !_DXGMX_VFS_H
