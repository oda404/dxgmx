/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_RAMFS_RAMFS_H
#define _DXGMX_FS_RAMFS_RAMFS_H

#include <dxgmx/fs/vfs.h>
#include <dxgmx/types.h>

typedef struct S_RamFsFileData
{
    void* data;
    bool used;
} RamFsFileData;

typedef struct S_RamFsMetadata
{
    RamFsFileData* files;
    size_t file_capacity;
    size_t file_cursor;
} RamFsMetadata;

int ramfs_init(FileSystem* fs);
void ramfs_destroy(FileSystem* fs);
VirtualNode* ramfs_vnode_lookup(const char* path, FileSystem* fs);

ERR_OR(ino_t)
ramfs_mkfile(
    VirtualNode* dir,
    const char* path,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    FileSystem* fs);

int ramfs_rmnode(VirtualNode* vnode);

int ramfs_open(VirtualNode* vnode, int flags);
ssize_t ramfs_read(const VirtualNode* vnode, void* buf, size_t n, off_t off);
int ramfs_ioctl(VirtualNode* vnode, int req, void* data);
ssize_t ramfs_write(VirtualNode* vnode, const void* buf, size_t n, off_t off);

#endif // !_DXGMX_FS_RAMFS_RAMFS_H
