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
#include <posix/sys/stat.h>

#define KLOGF_PREFIX "vfs: "

static FileSystemDriver* g_fs_drivers = NULL;
static size_t g_fs_drivers_count = 0;

static FileSystem* g_filesystems = NULL;
static size_t g_filesystems_count = 0;

/* The vfs is only responsible for setting the 'mntpoint', 'mntflags' and
 * 'driver' of a FileSystem. The rest is handled by the driver. This function
 * does not take in a 'driver' pointer as the driver may not be known yet. */
static FileSystem* vfs_new_fs(const char* mntpoint, u32 mntflags)
{
    char* newmntpoint = strdup(mntpoint);
    if (!newmntpoint)
        return NULL;

    /* Enlarge the filesystems array */
    FileSystem* tmp =
        krealloc(g_filesystems, (g_filesystems_count + 1) * sizeof(FileSystem));

    if (!tmp)
    {
        kfree(newmntpoint);
        return NULL;
    }

    g_filesystems = tmp;
    ++g_filesystems_count;

    /* Setup the filesystem */
    FileSystem* newfs = &g_filesystems[g_filesystems_count - 1];
    memset(newfs, 0, sizeof(FileSystem));

    newfs->mntpoint = newmntpoint;
    newfs->mntflags = mntflags;

    return newfs;
}

/* Remove a filesystem, freeing any resources that were allocated by the vfs. */
static int vfs_rm_fs_by_idx(size_t idx)
{
    ASSERT(idx < g_filesystems_count);

    FileSystem* fs = &g_filesystems[idx];

    /* Free any pointers that were allocated by the vfs. */
    if (fs->mntpoint)
        kfree(fs->mntpoint);

    /* Zero out the struct, just to make sure the space used by this fs
     * doesn't leak into any possible future filesystems. */
    memset(fs, 0, sizeof(FileSystem));

    /* Remove the filesystem from the array */
    for (size_t i = idx; i < g_filesystems_count - 1; ++i)
        g_filesystems[i] = g_filesystems[i + 1];

    --g_filesystems_count;

    /* Let's be nice and try to shrink the array. */
    FileSystem* tmp =
        krealloc(g_filesystems, g_filesystems_count * sizeof(FileSystem));

    if (tmp)
        g_filesystems = tmp;

    /* If !tmp we are left with one extra allocated filesystem that will get
     * caught by a krealloc next time we do vfs_new_fs. */

    return 0;
}

static int vfs_rm_fs(FileSystem* fs)
{
    /* We could also check for bounds and alignemnt... */
    for (size_t i = 0; i < g_filesystems_count; ++i)
    {
        if (&g_filesystems[i] == fs)
            return vfs_rm_fs_by_idx(i);
    }

    return -ENOENT;
}

static FileSystemDriver* vfs_new_fs_driver()
{
    FileSystemDriver* tmp = krealloc(
        g_fs_drivers, (g_fs_drivers_count + 1) * sizeof(FileSystemDriver));

    if (!tmp)
        return NULL;

    g_fs_drivers = tmp;
    ++g_fs_drivers_count;

    FileSystemDriver* fs_driver = &g_fs_drivers[g_fs_drivers_count - 1];
    memset(fs_driver, 0, sizeof(FileSystemDriver));
    return fs_driver;
}

/**
 * @brief Gets the FileSystem on which 'path' resides.
 * @return The filesystem on success, NULL represents an ENOMEM.
 */
static FileSystem* vfs_topmost_fs_for_path(const char* path)
{
    /* Only allow absolute paths */
    ASSERT(path && path[0] == '/');

    char* pathdup = strdup(path);
    if (!pathdup)
        return NULL;

    FileSystem* fshit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        strcpy(pathdup, path);
        ssize_t pathdup_len = strlen(pathdup);

        while (pathdup_len >= 1)
        {
            if (fs->mntpoint && strcmp(fs->mntpoint, pathdup) == 0)
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

    /* We should at least hit the filesystem mounted on / */
    ASSERT(fshit);

    return fshit;
}

_INIT int vfs_init()
{
    int st = vfs_mount("hdap0", "/", NULL, NULL, 0);
    if (st < 0)
        panic("Failed to mount / %d :(", st);

    vfs_mount(NULL, "/bin/a", "ramf", NULL, 0);

    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
        KLOGF(INFO, "%s on %s", fs->mntsrc, fs->mntpoint);

    return 0;
}

int vfs_mount(
    const char* src,
    const char* dest,
    const char* type,
    const char* args,
    u32 flags)
{
    if (!dest)
        return -EINVAL;

    FileSystem* fs = vfs_new_fs(dest, flags);
    if (!fs)
        return -ENOMEM;

    int st;

    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_drivers_count, driver)
    {
        /* Set the driver to the one we are probing. */
        fs->driver = driver;

        /* If the driver succeeded in initializing the filesystem, we know it's
         * valid and also up :) */
        st = driver->init(src, type, args, fs);
        if (st == 0)
            break;
    }

    if (st < 0)
    {
        vfs_rm_fs(fs);
        return st;
    }

    return 0;
}

int vfs_unmount(const char* src_or_mountpoint)
{
    if (!src_or_mountpoint)
        return -EINVAL;

    // TODO: Further validation

    FileSystem* fs_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        if ((fs->mntsrc && strcmp(fs->mntsrc, src_or_mountpoint) == 0) ||
            (fs->mntpoint && strcmp(fs->mntpoint, src_or_mountpoint) == 0))
        {
            fs_hit = fs;
            break;
        }
    }

    if (!fs_hit)
        return -ENOENT;

    fs_hit->driver->destroy(fs_hit);

    vfs_rm_fs(fs_hit);

    return 0;
}

int vfs_register_fs_driver(FileSystemDriver fs_driver)
{
    if (!(fs_driver.name && fs_driver.init && fs_driver.destroy &&
          fs_driver.read))
    {
        return -EINVAL;
    }

    /* Check to see if there is already a driver with the same name. */
    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_drivers_count, ops)
    {
        if (strcmp(ops->name, fs_driver.name) == 0)
            return -EEXIST;
    }

    FileSystemDriver* newfs_driver = vfs_new_fs_driver();
    if (!newfs_driver)
        return -ENOMEM;

    *newfs_driver = fs_driver;

    KLOGF(INFO, "Registered filesystem: '%s'.", newfs_driver->name);

    return 0;
}

int vfs_unregister_fs_driver(const char* name)
{
    if (!name)
        return -EINVAL;

    FileSystemDriver* fsdriver_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_drivers_count, fsdriver)
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
         d < g_fs_drivers + g_fs_drivers_count - 1;
         ++d)
        *d = *(d + 1);

    --g_fs_drivers_count;

    return 0;
}

int vfs_open(const char* name, int flags, mode_t mode)
{
    (void)flags;

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
    tmpfd.pid = 0;

    /* get a new fd for the new file */
    tmpfd.fd = vfs_next_free_fd_for_pid(0);
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

ssize_t vfs_read(int fd, void* buf, size_t n)
{
    if (!buf || !n)
    {
        errno = EINVAL;
        return -1;
    }

    OpenFileDescriptor* openfd = NULL;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, tmpfd)
    {
        if (tmpfd->fd == fd)
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

ssize_t vfs_write(int fd, const void* buf, size_t n)
{
    if (!buf || !n)
    {
        errno = EINVAL;
        return -1;
    }

    OpenFileDescriptor* openfd = NULL;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, tmpfd)
    {
        if (tmpfd->fd == fd)
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

off_t vfs_lseek(int fd, off_t off, int whence)
{
    OpenFileDescriptor* openfd = NULL;
    FOR_EACH_ELEM_IN_DARR (g_openfds, g_openfds_count, tmpfd)
    {
        if (tmpfd->fd == fd)
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

int vfs_close(int fd)
{
    (void)fd;
    return 0;
}
