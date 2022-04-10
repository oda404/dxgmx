/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/panic.h>
#include <dxgmx/stdio.h>
#include <dxgmx/storage/blkdevmanager.h>
#include <dxgmx/string.h>

#define KLOGF_PREFIX "vfs: "

static FileSystemOperations* g_filesystem_ops = NULL;
static size_t g_filesystem_ops_count = 0;

static FileSystem* g_filesystems = NULL;
static size_t g_filesystems_count = 0;

static FileSystem* vfs_new_fs()
{
    FileSystem* tmp =
        krealloc(g_filesystems, (g_filesystems_count + 1) * sizeof(FileSystem));

    if (!tmp)
        return NULL;

    g_filesystems = tmp;
    ++g_filesystems_count;

    memset(&g_filesystems[g_filesystems_count - 1], 0, sizeof(FileSystem));
    return &g_filesystems[g_filesystems_count - 1];
}

_INIT bool vfs_init()
{
    if (vfs_mount("hdap0", "/", 0) < 0)
        panic("Failed to mount / :(");

    return true;
}

int vfs_mount(const char* src, const char* dest, u32 flags)
{
    if (!src || !dest)
        return -EINVAL;

    const BlockDevice* blkdev = blkdevmanager_find_blkdev_by_name(src);

    if (!blkdev)
        return -ENOTBLK;

    if (blkdev->uuid)
        return vfs_mount_by_uuid(blkdev->uuid, dest, flags);

    return -ENOENT;
}

int vfs_mount_by_uuid(const char* uuid, const char* dest, u32 flags)
{
    if (!uuid || !dest)
        return -EINVAL;

    const BlockDevice* blkdev = blkdevmanager_find_blkdev_by_uuid(uuid);
    if (!blkdev)
        return -ENOTBLK;

    /* Try to find a suitable driver */
    FileSystemOperations* fsops_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_filesystem_ops, g_filesystem_ops_count, fsimpl)
    {
        if (fsimpl->valid && fsimpl->valid(blkdev))
        {
            fsops_hit = fsimpl;
            break;
        }
    }

    if (!fsops_hit)
        return -ENODEV;

    FileSystem* fs = vfs_new_fs();
    if (!fs)
        return -ENOMEM;

    fs->operations = fsops_hit;
    fs->mountflags = flags;
    fs->backing.blkdev = blkdev;

    return fs->operations->init(fs);
}

int vfs_unmount(const char* src_or_dest)
{
    if (!src_or_dest)
        return -EINVAL;

    size_t idx = 0;
    bool found = false;
    for (; idx < g_filesystems_count; ++idx)
    {
        FileSystem* tmp = &g_filesystems[idx];
        if ((tmp->mountsrc && strcmp(tmp->mountsrc, src_or_dest) == 0) ||
            (tmp->mountpoint && strcmp(tmp->mountpoint, src_or_dest) == 0))
        {
            found = true;
            break;
        }
    }

    if (!found)
        return -ENOENT;

    FileSystem* fs = &g_filesystems[idx];
    fs->operations->destroy(fs);

    if (fs->mountsrc)
        kfree(fs->mountsrc);

    if (fs->mountpoint)
        kfree(fs->mountpoint);

    for (size_t i = idx; i < g_filesystems_count - 1; ++i)
        g_filesystems[i] = g_filesystems[i + 1];

    --g_filesystems_count;

    return 0;
}

int vfs_register_fs_operations(const FileSystemOperations* fsops)
{
    if (!(fsops && fsops->name && fsops->valid && fsops->init &&
          fsops->destroy && fsops->read))
    {
        return -EINVAL;
    }

    /* Check to see if there is already a driver with the same name. */
    FOR_EACH_ELEM_IN_DARR (g_filesystem_ops, g_filesystem_ops_count, ops)
    {
        if (strcmp(ops->name, fsops->name) == 0)
            return -EEXIST;
    }

    FileSystemOperations* tmp = krealloc(
        g_filesystem_ops,
        (g_filesystem_ops_count + 1) * sizeof(FileSystemOperations));

    if (!tmp)
        return -ENOMEM;

    g_filesystem_ops = tmp;
    ++g_filesystem_ops_count;

    g_filesystem_ops[g_filesystem_ops_count - 1] = *fsops;

    KLOGF(INFO, "Registered filesystem: '%s'.", fsops->name);

    return 0;
}

int vfs_unregister_fs_operations(const char* name)
{
    if (!name)
        return -EINVAL;

    size_t fsops_idx = 0;

    {
        /* Find the filesystem operations struct */
        bool found = false;
        for (; fsops_idx < g_filesystem_ops_count; ++fsops_idx)
        {
            FileSystemOperations* fsops = &g_filesystem_ops[fsops_idx];
            if (fsops->name && strcmp(fsops->name, name) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
            return -ENOENT;
    }

    {
        /* Check to see if it's in use by any filesystems */
        bool inuse = false;
        const FileSystemOperations* fsops = &g_filesystem_ops[fsops_idx];
        FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
        {
            /* We could also strcmp names ... */
            if (fs->operations == fsops)
            {
                inuse = true;
                break;
            }
        }

        if (inuse)
            return -EBUSY;
    }

    /* If we got here the FileSystemOperations is valid, and it's not
    in use by any mounted filesystems, so we remove it. */
    for (size_t i = fsops_idx; i < g_filesystem_ops_count - 1; ++i)
        g_filesystem_ops[i] = g_filesystem_ops[i + 1];

    --g_filesystem_ops_count;

    return 0;
}
