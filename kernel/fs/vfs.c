/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/openfd.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/limits.h>
#include <dxgmx/panic.h>
#include <dxgmx/stdio.h>
#include <dxgmx/storage/blkdevmanager.h>
#include <dxgmx/string.h>
#include <posix/fcntl.h>
#include <posix/sys/stat.h>

#define KLOGF_PREFIX "vfs: "

static FileSystemDriver* g_filesystem_drivers = NULL;
static size_t g_filesystem_drivers_count = 0;

static FileSystem* g_filesystems = NULL;
static size_t g_filesystems_count = 0;

static OpenFileDescriptor* g_openfds = NULL;
static size_t g_openfds_count = 0;

/* Returns the next available file descriptor number for the given PID.
A negative value means no file descriptors are available. */
static int vfs_next_free_fd_for_pid(pid_t pid)
{
    int fd = 3;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, openfd)
        fd += (openfd->pid == pid);
    return fd;
}

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

static int vfs_rm_fs(FileSystem* fs)
{
    if (!fs || !g_filesystems)
        return -EINVAL;

    bool hit = false;
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, f)
    {
        if (f == fs)
        {
            hit = true;
            break;
        }
    }

    if (!hit)
        return -ENOENT;

    /* 'driver_ctx' and 'vnodes' are not freed here because they were the
     * responsability of the filesystem driver, and they might as well just be
     * statically allocated. */

    if (fs->mountsrc)
        kfree(fs->mountsrc);

    if (fs->mountpoint)
        kfree(fs->mountpoint);

    for (FileSystem* f = fs; f < g_filesystems + g_filesystems_count - 1; ++f)
        *f = *(f + 1);

    --g_filesystems_count;

    return 0;
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

static OpenFileDescriptor* vfs_new_openfd()
{
    OpenFileDescriptor* tmp =
        krealloc(g_openfds, (g_openfds_count + 1) * sizeof(OpenFileDescriptor));

    if (!tmp)
        return NULL;

    g_openfds = tmp;
    ++g_openfds_count;

    OpenFileDescriptor* openfd = &g_openfds[g_openfds_count - 1];
    memset(openfd, 0, sizeof(OpenFileDescriptor));

    return openfd;
}

static int vfs_rm_openfd(OpenFileDescriptor* openfd)
{
    if (!openfd)
        return -EINVAL;

    bool hit = false;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, fd)
    {
        if (fd == openfd)
        {
            hit = true;
            break;
        }
    }

    if (!hit)
        return -ENOENT;

    for (OpenFileDescriptor* f = openfd; f < g_openfds + g_openfds_count - 1;
         ++f)
        *f = *(f + 1);

    OpenFileDescriptor* tmp =
        krealloc(g_openfds, (g_openfds_count - 1) * sizeof(OpenFileDescriptor));

    ASSERT(tmp);

    g_openfds = tmp;
    --g_openfds_count;

    return 0;
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

_INIT bool vfs_init()
{
    if (vfs_mount_ramfs("ramfs", "/", 0) < 0)
        panic("Failed to mount ramfs on / :(");

    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
        KLOGF(INFO, "%s on %s", fs->mountsrc, fs->mountpoint);

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
    if (!blkdev || !blkdev->name)
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

    FileSystem tmp = {
        .backing.blkdev = blkdev,
        .mountflags = flags,
        .mountpoint = strdup(dest),
        .mountsrc = strdup(blkdev->name),
        .driver = fs_driver_hit,
        .driver_ctx = NULL,
        .vnodes = NULL,
        .vnode_count = 0};

    FileSystem* fs = NULL;
    int st = 0;

    if (!tmp.mountpoint || !tmp.mountsrc)
    {
        st = -ENOMEM;
        goto fail;
    }

    if (!(fs = vfs_new_fs()))
    {
        st = -ENOMEM;
        goto fail;
    }

    *fs = tmp;

    if ((st = fs->driver->init(fs)) != 0)
        goto fail;

    return st;

fail:
    if (fs)
        vfs_rm_fs(fs);

    return st;
}

int vfs_unmount(const char* src_or_mountpoint)
{
    if (!src_or_mountpoint)
        return -EINVAL;

    FileSystem* fs_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        if ((fs->mountsrc && strcmp(fs->mountsrc, src_or_mountpoint) == 0) ||
            (fs->mountpoint && strcmp(fs->mountpoint, src_or_mountpoint) == 0))
        {
            fs_hit = fs;
            break;
        }
    }

    if (!fs_hit)
        return -ENOENT;

    fs_hit->driver->destroy(fs_hit);

    if (fs_hit->mountsrc)
        kfree(fs_hit->mountsrc);

    if (fs_hit->mountpoint)
        kfree(fs_hit->mountpoint);

    for (FileSystem* fs = fs_hit; fs < g_filesystems + g_filesystems_count - 1;
         ++fs)
    {
        *fs = *(fs + 1);
    }

    --g_filesystems_count;

    return 0;
}

int vfs_mount_ramfs(const char* driver_name, const char* dest, u32 flags)
{
    if (!driver_name || !dest)
        return -EINVAL;

    /* Try to find a suitable driver */
    FileSystemDriver* fs_driver_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (
        g_filesystem_drivers, g_filesystem_drivers_count, fsdriver)
    {
        if (fsdriver->backing == FILESYSTEM_BACKING_RAM &&
            strcmp(fsdriver->name, driver_name) == 0)
        {
            fs_driver_hit = fsdriver;
            break;
        }
    }

    if (!fs_driver_hit)
        return -ENODEV;

    FileSystem tmp = {
        .mountflags = flags,
        .mountpoint = strdup(dest),
        .mountsrc = strdup(driver_name),
        .driver = fs_driver_hit,
        .driver_ctx = NULL,
        .vnodes = NULL,
        .vnode_count = 0};

    FileSystem* fs = NULL;
    int st = 0;

    if (!tmp.mountpoint || !tmp.mountsrc)
    {
        st = -ENOMEM;
        goto fail;
    }

    if (!(fs = vfs_new_fs()))
    {
        st = -ENOMEM;
        goto fail;
    }

    *fs = tmp;

    if ((st = fs->driver->init(fs)) != 0)
        goto fail;

    return 0;

fail:
    if (fs)
        vfs_rm_fs(fs);

    return st;
}

int vfs_register_fs_driver(const FileSystemDriver* fs_driver)
{
    if (!(fs_driver && fs_driver->name && fs_driver->init &&
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

    FileSystemDriver* fsdriver_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (
        g_filesystem_drivers, g_filesystem_drivers_count, fsdriver)
    {
        if (fsdriver->name && strcmp(fsdriver->name, name) == 0)
        {
            fsdriver_hit = fsdriver;
            break;
        }
    }

    if (!fsdriver_hit)
        return -ENOENT;

    /* Check to see if it's in use by any filesystems */
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        /* We could also strcmp names ... */
        if (fs->driver == fsdriver_hit)
            return -EBUSY;
    }

    /* If we got here the FileSystemDriver is valid, and it's not
    in use by any mounted filesystems, so we remove it. */
    for (FileSystemDriver* d = fsdriver_hit;
         d < g_filesystem_drivers + g_filesystem_drivers_count - 1;
         ++d)
        *d = *(d + 1);

    --g_filesystem_drivers_count;

    return 0;
}

int vfs_open(const char* name, int flags, mode_t mode, pid_t pid)
{
    (void)flags;

    ASSERT(pid == 0);

    if (!name)
    {
        errno = EINVAL;
        return -1;
    }

    FileSystem* fs = vfs_topmost_fs_for_path(name);
    if (!fs)
    {
        errno = EINVAL;
        return -1;
    }

    OpenFileDescriptor tmpfd;
    tmpfd.mode = mode;
    tmpfd.flags = flags;
    tmpfd.off = 0;
    tmpfd.pid = pid;

    /* get a new fd for the new file */
    tmpfd.fd = vfs_next_free_fd_for_pid(pid);
    if (tmpfd.fd < 0)
    {
        errno = ERANGE;
        return -1;
    }

    /* create the open file descriptor */
    OpenFileDescriptor* openfd = vfs_new_openfd();
    if (!openfd)
    {
        errno = ENOMEM;
        return -1;
    }

    /* Get the vnode for the path. */
    tmpfd.vnode = fs_vnode_for_path(fs, name);
    if (!tmpfd.vnode)
    {
        if (!(flags & O_CREAT))
            return -1;

        errno = 0;
        if (fs->driver->mkfile(fs, name, mode) < 0)
        {
            vfs_rm_openfd(openfd);
            return -1;
        }

        tmpfd.vnode = fs_vnode_for_path(fs, name);
        ASSERT(tmpfd.vnode);
    }
    else if (tmpfd.vnode->mode & S_IFDIR)
    {
        vfs_rm_openfd(openfd);
        errno = EISDIR;
        return -1;
    }

    *openfd = tmpfd;

    return tmpfd.fd;
}

ssize_t vfs_read(int fd, void* buf, size_t n, pid_t pid)
{
    if (!buf || !n)
    {
        errno = EINVAL;
        return -1;
    }

    OpenFileDescriptor* openfd = NULL;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, tmpfd)
    {
        if (tmpfd->fd == fd && tmpfd->pid == pid)
        {
            openfd = tmpfd;
            break;
        }
    }

    if (!openfd || !openfd->vnode)
    {
        errno = ENOENT;
        return -1;
    }

    if (!(openfd->flags & O_RDONLY))
    {
        errno = EPERM;
        return -1;
    }

    FileSystem* fs = openfd->vnode->owner;
    if (!fs)
    {
        errno = EINVAL;
        return -1;
    }

    ssize_t read = fs->driver->read(fs, openfd->vnode, buf, n, openfd->off);
    if (read < 0)
        return -1; /* errno should already be set by fs->driver->read(). */

    openfd->off += read;
    return read;
}

ssize_t vfs_write(int fd, const void* buf, size_t n, pid_t pid)
{
    if (!buf || !n)
    {
        errno = EINVAL;
        return -1;
    }

    OpenFileDescriptor* openfd = NULL;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, tmpfd)
    {
        if (tmpfd->fd == fd && tmpfd->pid == pid)
        {
            openfd = tmpfd;
            break;
        }
    }

    if (!openfd || !openfd->vnode)
    {
        errno = ENOENT;
        return -1;
    }

    if (!(openfd->flags & O_WRONLY))
    {
        errno = EPERM;
        return -1;
    }

    FileSystem* fs = openfd->vnode->owner;
    if (!fs)
    {
        errno = EINVAL;
        return -1;
    }

    const ssize_t written =
        fs->driver->write(fs, openfd->vnode, buf, n, openfd->off);

    if (written < 0)
        return -1; /* errno should already be set by fs->driver->write(). */

    openfd->off += written;
    return written;
}

off_t vfs_lseek(int fd, off_t off, int whence, pid_t pid)
{
    OpenFileDescriptor* openfd = NULL;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, tmpfd)
    {
        if (tmpfd->fd == fd && tmpfd->pid == pid)
        {
            openfd = tmpfd;
            break;
        }
    }

    if (!openfd || !openfd->vnode)
    {
        errno = ENOENT;
        return -1;
    }

    if (whence == SEEK_SET)
    {
        openfd->off = off;
    }
    else if (whence == SEEK_CUR)
    {
        openfd->off += off;
    }
    else if (whence == SEEK_END)
    {
        openfd->off = openfd->vnode->size + off;
    }
    else
    {
        errno = EINVAL;
        return -1;
    }

    return openfd->off;
}

int vfs_close(int fd, pid_t pid)
{
    (void)fd;
    (void)pid;
    return 0;
}
