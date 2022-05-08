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

static FileSystemDriver* g_filesystem_drivers = NULL;
static size_t g_filesystem_drivers_count = 0;

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

static FileSystemDriver* vfs_new_fs_driver()
{
    FileSystemDriver* tmp = krealloc(
        g_filesystem_drivers,
        (g_filesystem_drivers_count + 1) * sizeof(FileSystemDriver));

    if (!tmp)
        return NULL;

    g_filesystem_drivers = tmp;
    ++g_filesystem_drivers_count;

    FileSystemDriver* fs_driver =
        &g_filesystem_drivers[g_filesystem_drivers_count - 1];
    memset(fs_driver, 0, sizeof(FileSystemDriver));
    return fs_driver;
}

/**
 * @brief Gets the FileSystem on which 'path' resides.
 * @return The fileystem, if NULL errno indicates the error.
 */
static FileSystem* vfs_topmost_fs_for_path(const char* path)
{
    /* We only allow absolute paths. */
    if (!path || path[0] != '/')
    {
        errno = EINVAL;
        return NULL;
    }

    char* pathdup = strdup(path);
    if (!pathdup)
    {
        errno = ENOMEM;
        return NULL;
    }

    FileSystem* fshit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        strcpy(pathdup, path);
        ssize_t pathdup_len = strlen(pathdup);

        while (pathdup_len >= 1)
        {
            if (fs->mountpoint && strcmp(fs->mountpoint, pathdup) == 0)
            {
                fshit = fs;
                break;
            }

            for (ssize_t i = pathdup_len - 1; i >= 0; --i)
            {
                if (pathdup[i] == '/')
                {
                    if (i == pathdup_len - 1)
                        pathdup[i] = '\0';
                    else
                        pathdup[i + 1] = '\0';
                    break;
                }
            }

            pathdup_len = strlen(pathdup);
        }
    }

    kfree(pathdup);

    if (!fshit)
        errno = ENOENT;

    return fshit;
}

/**
 * @brief Gets the vnode for the given path.
 * @return The vnode, if NULL errno indicates the error.
 */
static VirtualNode* vfs_vnode_for_path(const char* path)
{
    errno = 0;

    if (!path)
    {
        errno = EINVAL;
        return NULL;
    }

    FileSystem* fs = vfs_topmost_fs_for_path(path);
    if (!fs)
        return NULL; /* we keep the errno from vfs_topmost_fs_for_path(). */

    if (!fs->driver || !fs->driver->vnode_for_path)
    {
        errno = EINVAL;
        return NULL;
    }

    char* working_path = strdup(path);
    if (!working_path)
    {
        errno = ENOMEM;
        return NULL;
    }

    /* Chop of the prefix, leaving a path relative to the filesystem's
     * mountpoint */
    size_t prefixlen = strlen(fs->mountpoint);
    while (prefixlen)
    {
        const size_t len = strlen(working_path);
        for (size_t i = 0; i < len; ++i)
            working_path[i] = working_path[i + 1];

        --prefixlen;
    }

    VirtualNode* vnode = fs->driver->vnode_for_path(fs, working_path);

    kfree(working_path);

    return vnode;
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
    FileSystemDriver* fs_driver_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (
        g_filesystem_drivers, g_filesystem_drivers_count, fsimpl)
    {
        if (fsimpl->valid && fsimpl->valid(blkdev))
        {
            fs_driver_hit = fsimpl;
            break;
        }
    }

    if (!fs_driver_hit)
        return -ENODEV;

    FileSystem* fs = vfs_new_fs();
    if (!fs)
        return -ENOMEM;

    fs->operations = fsops_hit;
    fs->mountflags = flags;
    fs->backing.blkdev = blkdev;

    int st = fs->driver->init(fs);
    if (st != 0)
    {
        kfree(tmp.mountpoint);
        // delete new fs
    }

    return st;
}

int vfs_unmount(const char* src_or_dest)
{
    if (!src_or_dest)
        return -EINVAL;

    ssize_t idx = -1;
    for (size_t i = 0; i < g_filesystems_count; ++i)
    {
        FileSystem* tmp = &g_filesystems[i];
        if ((tmp->mountsrc && strcmp(tmp->mountsrc, src_or_dest) == 0) ||
            (tmp->mountpoint && strcmp(tmp->mountpoint, src_or_dest) == 0))
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
        return -ENOENT;

    FileSystem* fs = &g_filesystems[idx];
    fs->driver->destroy(fs);

    if (fs->mountsrc)
        kfree(fs->mountsrc);

    if (fs->mountpoint)
        kfree(fs->mountpoint);

    for (size_t i = idx; i < g_filesystems_count - 1; ++i)
        g_filesystems[i] = g_filesystems[i + 1];

    --g_filesystems_count;

    return 0;
}

int vfs_register_fs_driver(const FileSystemDriver* fs_driver)
{
    if (!(fs_driver && fs_driver->name && fs_driver->valid && fs_driver->init &&
          fs_driver->destroy && fs_driver->read))
    {
        return -EINVAL;
    }

    /* Check to see if there is already a driver with the same name. */
    FOR_EACH_ELEM_IN_DARR (
        g_filesystem_drivers, g_filesystem_drivers_count, ops)
    {
        if (strcmp(ops->name, fs_driver->name) == 0)
            return -EEXIST;
    }

    FileSystemDriver* newfs_driver = vfs_new_fs_driver();
    if (!newfs_driver)
        return -ENOMEM;

    *newfs_driver = *fs_driver;

    KLOGF(INFO, "Registered filesystem: '%s'.", newfs_driver->name);

    return 0;
}

int vfs_unregister_fs_driver(const char* name)
{
    if (!name)
        return -EINVAL;

    ssize_t fs_driver_idx = -1;
    /* Find the filesystem operations struct */
    for (size_t i = 0; i < g_filesystem_drivers_count; ++i)
    {
        FileSystemDriver* fs_driver = &g_filesystem_drivers[i];
        if (fs_driver->name && strcmp(fs_driver->name, name) == 0)
        {
            fs_driver_idx = i;
            break;
        }
    }

    if (fs_driver_idx == -1)
        return -ENOENT;

    /* Check to see if it's in use by any filesystems */
    const FileSystemDriver* fs_driver = &g_filesystem_drivers[fs_driver_idx];
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        /* We could also strcmp names ... */
        if (fs->driver == fs_driver)
            return -EBUSY;
    }

    /* If we got here the FileSystemDriver is valid, and it's not
    in use by any mounted filesystems, so we remove it. */
    for (size_t i = fs_driver_idx; i < g_filesystem_drivers_count - 1; ++i)
        g_filesystem_drivers[i] = g_filesystem_drivers[i + 1];

    --g_filesystem_drivers_count;

    return 0;
}

    return 0;
}
