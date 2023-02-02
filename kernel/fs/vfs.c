/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/**
 * Virtual filesystem implementation
 * A lot of data structures in here are trash but they work for now.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/fd.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/limits.h>
#include <dxgmx/panic.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/stdio.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bitwise.h>
#include <posix/sys/stat.h>

#define KLOGF_PREFIX "vfs: "

/**
 * Global file descriptor table.
 */
static FileDescriptor* g_sys_fds = NULL;
static size_t g_sys_fd_count = 0;

/* Registered filesystem drivers. */
static FileSystemDriver* g_fs_drivers = NULL;
static size_t g_fs_drivers_count = 0;

/* Mounted filesystems */
static FileSystem* g_filesystems = NULL;
static size_t g_filesystems_count = 0;

/**
 * Create a new system-wide open file description.
 * Retuns a FileDescriptor* on sucess, NULL on out memory.
 */
static FileDescriptor* vfs_new_file_descriptor()
{
    /* Try finding any allocated but unused fds. */
    for (size_t i = 0; i < g_sys_fd_count; ++i)
    {
        if (g_sys_fds[i].fd == 0)
            return &g_sys_fds[i];
    }

    /* If none were found, we allocate one. FIXME: maybe multiple ? */
    FileDescriptor* tmp =
        krealloc(g_sys_fds, (g_sys_fd_count + 1) * sizeof(FileDescriptor));

    if (!tmp)
        return NULL;

    g_sys_fds = tmp;
    ++g_sys_fd_count;

    tmp = &g_sys_fds[g_sys_fd_count - 1];
    memset(tmp, 0, sizeof(FileDescriptor));

    return tmp;
}

/* Destroy a system-wide file description. */
static int vfs_free_file_descriptor(FileDescriptor* fd)
{
    /* Check if fd is actually part of g_sys_fds */
    ASSERT(
        fd >= g_sys_fds && fd < g_sys_fds + g_filesystems_count &&
        (size_t)fd % sizeof(ptr) == 0);

    /**
     * We do not shrink nor shift the array. This is beacause:
     * 1) Files are often opened and closed, shrinking this array every time
     * would cause a lot of overhead
     * 2) This array is indexed by the local file descriptor table of every
     * process. Shifting the elements in this array would mean messing up those
     * offsets.
     */
    memset(fd, 0, sizeof(FileDescriptor));

    return 0;
}

/**
 * Create a new fileystem
 * 'mntpoint' is the mount destination.
 *
 * Returns a FilesSystem with 'mntpoint' set on sucess, NULL on out of memory.
 */
static FileSystem* vfs_new_fs(const char* mntpoint)
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

    return newfs;
}

/* Remove a filesystem, freeing any resources that were allocated by the vfs. */
static int vfs_free_fs_by_idx(size_t idx)
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

static int vfs_free_fs(FileSystem* fs)
{
    /* We could also check for bounds and alignemnt... */
    for (size_t i = 0; i < g_filesystems_count; ++i)
    {
        if (&g_filesystems[i] == fs)
            return vfs_free_fs_by_idx(i);
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

static FileSystem* vfs_find_fs_by_src_or_dest(const char* src_or_dest)
{
    FOR_EACH_ELEM_IN_DARR (g_filesystems, g_filesystems_count, fs)
    {
        if ((fs->mntsrc && strcmp(fs->mntsrc, src_or_dest) == 0) ||
            (fs->mntpoint && strcmp(fs->mntpoint, src_or_dest) == 0))
        {
            return fs;
        }
    }

    return NULL;
}

/**
 * Get the system-wide file descriptor for 'fd' and 'proc'.
 * 'fd' is a local file descriptor for the 'proc' process.
 * Does not do safety checks.
 */
static FileDescriptor* vfs_get_sysfd(fd_t fd, Process* proc)
{
    if (fd < 0 || (size_t)fd >= proc->fd_count)
        return NULL;

    size_t idx = proc->fds[fd];
    if (idx >= g_sys_fd_count)
        return NULL; // someone's doing shady stuff.

    return &g_sys_fds[idx];
}

/**
 * Transform a system wide file descriptor into it's index in the system wide fd
 * table.
 */
static size_t vfs_sysfd_to_idx(FileDescriptor* fd)
{
    ASSERT(
        fd >= g_sys_fds && fd < g_sys_fds + g_sys_fd_count &&
        (ptr)fd % sizeof(ptr) == 0);

    return (ptr)fd - (ptr)g_sys_fds;
}

_INIT int vfs_init()
{
    int st = vfs_mount("hdap0", "/", NULL, NULL, 0);
    if (st < 0)
        panic("Failed to mount / %d :(", st);

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

    /* Create new filesystem structure */
    FileSystem* fs = vfs_new_fs(dest);
    if (!fs)
        return -ENOMEM;

    /* The vfs only sets fs->mntpoint, fs->flags, and fs->driver. The rest is up
     * to the driver */
    fs->flags = flags;

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
        vfs_free_fs(fs);
        return st;
    }

    return 0;
}

int vfs_unmount(const char* src_or_dest)
{
    if (!src_or_dest)
        return -EINVAL;

    FileSystem* fs = vfs_find_fs_by_src_or_dest(src_or_dest);
    if (!fs)
        return -ENOENT;

    /* Cleanup driver */
    fs->driver->destroy(fs);

    /* Cleanup vfs */
    vfs_free_fs(fs);

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
    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_drivers_count, driv)
    {
        if (strcmp(driv->name, fs_driver.name) == 0)
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

int vfs_open(const char* path, int flags, mode_t mode, Process* proc)
{
    if (!path || !proc)
        return -EINVAL;

    FileSystem* fs = vfs_topmost_fs_for_path(path);
    if (!fs)
        return -ENOMEM;

    VirtualNode* vnode = fs_vnode_for_path(fs, path);
    if (!vnode)
    {
        /* Try to create it */
        if (flags & O_CREAT)
        {
            int st = vnode->owner->driver->mkfile(vnode->owner, path, mode);
            if (st < 0)
                return st;
        }

        /* Try again, this time it should work */
        vnode = fs_vnode_for_path(fs, path);
        ASSERT(vnode);
    }

    /* Create a new system-wide file descriptor */
    FileDescriptor* sysfd = vfs_new_file_descriptor();
    if (!sysfd)
        return -ENOMEM;

    /* Get current process and create a new process fd */
    int procfd = proc_new_fd(vfs_sysfd_to_idx(sysfd), proc);
    if (procfd < 0)
    {
        /* FIXME: delete file if it was created with O_CREAT */
        vfs_free_file_descriptor(sysfd);
        return procfd;
    }

    if ((flags & O_RDWR) && (flags & O_TRUNC))
        TODO();

    /* Fill out sysfd */
    sysfd->fd = procfd;
    sysfd->pid = proc->pid;
    sysfd->off = 0;
    sysfd->flags = flags;
    sysfd->mode = mode;
    sysfd->vnode = vnode;

    return sysfd->fd;
}

ssize_t vfs_read(fd_t fd, void* buf, size_t n, Process* proc)
{
    if (!buf || !n || !proc)
        return -EINVAL;

    FileDescriptor* sysfd = vfs_get_sysfd(fd, proc);
    if (!sysfd)
        return -ENOENT;

    if (!BW_MASK(sysfd->flags, O_RDONLY))
        return -EPERM;

    FileSystem* fs = sysfd->vnode->owner;
    if (!fs)
        return -EINVAL;

    ssize_t read = fs->driver->read(fs, sysfd->vnode, buf, n, sysfd->off);
    if (read < 0)
        return -1; /* errno should already be set by fs->driver->read(). */

    sysfd->off += read;
    return read;
}

ssize_t vfs_write(fd_t fd, const void* buf, size_t n, Process* proc)
{
    if (!buf || !n || !proc)
        return -EINVAL;

    FileDescriptor* sysfd = vfs_get_sysfd(fd, proc);
    if (!sysfd)
        return -ENOENT;

    if (!BW_MASK(sysfd->flags, O_WRONLY))
        return -EPERM;

    FileSystem* fs = sysfd->vnode->owner;
    if (!fs)
        return -EINVAL;

    /* Set offset to the end of the file if appending */
    if (sysfd->flags & O_APPEND)
        sysfd->off = sysfd->vnode->size;

    ssize_t written = fs->driver->write(fs, sysfd->vnode, buf, n, sysfd->off);
    if (written < 0)
        return -1; /* FIXME: error is in errno */

    sysfd->off += written;
    return written;
}

off_t vfs_lseek(fd_t fd, off_t off, int whence, Process* proc)
{
    if (!proc)
        return -EINVAL;

    FileDescriptor* sysfd = vfs_get_sysfd(fd, proc);
    if (!sysfd)
        return -ENOENT;

    if (whence == SEEK_SET)
        sysfd->off = off;
    else if (whence == SEEK_CUR)
        sysfd->off += off;
    else if (whence == SEEK_END)
        sysfd->off = sysfd->vnode->size + off;
    else
        return -EINVAL;

    return sysfd->off;
}

int vfs_close(fd_t fd, Process* proc)
{
    if (!proc)
        return -EINVAL;

    size_t idx = proc_free_fd(fd, proc);
    if (idx == PLATFORM_MAX_UNSIGNED)
        return -EINVAL;

    ASSERT(idx < g_sys_fd_count);

    return vfs_free_file_descriptor(&g_sys_fds[idx]);
}
