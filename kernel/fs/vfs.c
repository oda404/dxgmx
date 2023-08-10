/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/**
 * Virtual filesystem implementation
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
#include <dxgmx/sched/sched.h>
#include <dxgmx/stdio.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>
#include <dxgmx/utils/bitwise.h>
#include <dxgmx/utils/hashtable.h>
#include <posix/sys/stat.h>

#define KLOGF_PREFIX "vfs: "

/**
 * Global file descriptor table.
 */
static FileDescriptor* g_sys_fds = NULL;
static size_t g_sys_fd_count = 0;

/* Registered filesystem drivers. */
static const FileSystemDriver** g_fs_drivers = NULL;
static size_t g_fs_driver_count = 0;

/* Linked list of all the mounted filesystems. The reason for using a linked
 * list, is that filesystems are referenced by their pointers in a lot of
 * places, which means that if we hold them in an array, and later need to
 * enlarge that array with krealloc, that block of memory might change it's
 * starting point, leaving all references dangling. */
static LinkedList g_filesystems_ll;

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
        fd >= g_sys_fds && fd < g_sys_fds + g_sys_fd_count &&
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
 * 'mntsrc' Non-NULL mount source.
 * 'mntpoint' Non-NULL mount destination.
 * 'args' Arguments for the filesystem driver. May be null.
 *
 * Returns:
 * A FileSystem* with mntsrc, mntpoint and args set on success.
 * NULL on out of memory.
 */
static FileSystem*
vfs_new_fs(const char* mntsrc, const char* mntpoint, const char* args)
{
    FileSystem* newfs = kcalloc(sizeof(FileSystem));
    if (!newfs)
        return NULL;

    newfs->mntsrc = strdup(mntsrc);
    if (!newfs->mntsrc)
        goto out_err;

    newfs->mntpoint = strdup(mntpoint);
    if (!newfs->mntpoint)
        goto out_err;

    if (args)
    {
        newfs->args = strdup(args);
        if (!newfs->args)
            goto out_err;
    }

    if (linkedlist_add(newfs, &g_filesystems_ll) < 0)
        goto out_err;

    return newfs;

out_err:
    if (newfs->mntsrc)
        kfree((void*)newfs->mntsrc);

    if (newfs->mntpoint)
        kfree((void*)newfs->mntpoint);

    if (newfs->args)
        kfree((void*)newfs->args);

    kfree(newfs);

    return NULL;
}

static int vfs_free_fs(FileSystem* fs)
{
    int st = linkedlist_remove_by_data(fs, &g_filesystems_ll);

    if (st == 0)
    {
        kfree((void*)fs->mntsrc);
        kfree((void*)fs->mntpoint);

        if (fs->args)
            kfree((void*)fs->args);

        kfree(fs);
    }

    return 0;
}

static const FileSystemDriver** vfs_new_fs_driver()
{
    const FileSystemDriver** tmp = krealloc(
        g_fs_drivers, (g_fs_driver_count + 1) * sizeof(FileSystemDriver*));

    if (!tmp)
        return NULL;

    g_fs_drivers = tmp;
    ++g_fs_driver_count;

    const FileSystemDriver** fs_driver = &g_fs_drivers[g_fs_driver_count - 1];
    return fs_driver;
}

/**
 * Get the topmost filesystem on which path resides, basically the filesystem
 * that owns path. This function takes into account shadowing, for example:
 * If we mount hdap0 on / and then also mount a ramfs on /, the ramfs will be
 * returned due to shadowing.
 *
 * 'path' Non NULL path.
 *
 * Returns:
 * A non NULL FileSystem*. Note that "at least" the filesystem mounted on / will
 * be returned.
 */
static FileSystem* vfs_topmost_fs_for_path(const char* path)
{
    FileSystem* fs_hit = NULL;
    FOR_EACH_ENTRY_IN_LL (g_filesystems_ll, FileSystem*, fs)
    {
        const char* hit = strstr(path, fs->mntpoint);
        if (hit == path)
        {
            if (!fs_hit)
            {
                fs_hit = fs;
                continue;
            }

            size_t cur_len = strlen(fs_hit->mntpoint);
            size_t new_len = strlen(fs->mntpoint);

            /* We do >= because filesystems are always in mount ordrer,
             * and we want to take into account shadowing. */
            if (new_len >= cur_len)
                fs_hit = fs;
        }
    }

    /* We should at least hit the filesystem mounted on / */
    ASSERT(fs_hit);

    return fs_hit;
}

static FileSystem* vfs_find_fs_by_src_or_dest(const char* src_or_dest)
{
    FOR_EACH_ENTRY_IN_LL (g_filesystems_ll, FileSystem*, fs)
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
        return NULL; // somethings wrong ?

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
    linkedlist_init(&g_filesystems_ll);

    int st = vfs_mount("hdap0", "/", NULL, NULL, 0);
    if (st < 0)
        panic("Failed to mount / %d :(", st);

    st = vfs_mount("devfs", "/dev", "devfs", NULL, 0);

    FOR_EACH_ENTRY_IN_LL (g_filesystems_ll, FileSystem*, fs)
        KLOGF(INFO, "%s on %s", fs->mntsrc, fs->mntpoint);

    return 0;
}

int vfs_mount(
    const char* _USERPTR src,
    const char* _USERPTR dest,
    const char* _USERPTR type,
    const char* _USERPTR args,
    u32 flags)
{
    FileSystem* fs = NULL;
    {
        char safe_src[PATH_MAX];
        char safe_dest[PATH_MAX];
        char safe_args[PATH_MAX];
        if (user_copy_str_from(src, safe_src, PATH_MAX) < 0)
            return -EFAULT;
        if (user_copy_str_from(dest, safe_dest, PATH_MAX) < 0)
            return -EFAULT;
        if (args && user_copy_str_from(args, safe_args, PATH_MAX) < 0)
            return -EFAULT;

        fs = vfs_new_fs(safe_src, safe_dest, safe_args);
        if (!fs)
            return -ENOMEM;
    }

    fs->flags = flags;

    char safe_type[PATH_MAX];
    if (type && user_copy_str_from(type, safe_type, PATH_MAX) < 0)
    {
        vfs_free_fs(fs);
        return -EFAULT;
    }

    int st = -ENOENT;
    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_driver_count, driver)
    {
        /* Set the driver to the one we are (possibly) probing. */
        fs->driver = *driver;
        st = -ENOENT;

        /* Look for and probe the corresponding driver if type is not NULL. */
        if (type)
        {
            if (strcmp((*driver)->name, safe_type) == 0)
            {
                st = (*driver)->init(fs);
                goto out;
            }
        }
        else if ((*driver)->generic_probe)
        {
            st = (*driver)->init(fs);
        }

        /* If a generic probe returns 0 it means we succeeded. Drivers return
         * -ENOENT if the driver doesn't understant that filesystem. Which means
         * that if our error is != -ENOENT the driver understood the filesystem
         * but failed to initialize it. */
        if (st == 0 || st != -ENOENT)
            break;
    }

out:
    if (st < 0)
        vfs_free_fs(fs);

    return st;
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

int vfs_register_fs_driver(const FileSystemDriver* driver)
{
    /* Check to see if there is already a driver with the same name. */
    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_driver_count, drv)
    {
        if (strcmp((*drv)->name, driver->name) == 0)
            return -EEXIST;
    }

    const FileSystemDriver** newfs_driver = vfs_new_fs_driver();
    if (!newfs_driver)
        return -ENOMEM;

    *newfs_driver = driver;

    KLOGF(INFO, "Registered filesystem: '%s'.", (*newfs_driver)->name);

    return 0;
}

int vfs_unregister_fs_driver(const FileSystemDriver* driver)
{
    const FileSystemDriver** driver_hit = NULL;
    FOR_EACH_ELEM_IN_DARR (g_fs_drivers, g_fs_driver_count, drv)
    {
        if (*drv == driver)
        {
            driver_hit = drv;
            break;
        }
    }

    if (!driver_hit)
        return -ENOENT;

    /* Check to see if it's in use by any filesystems */
    FOR_EACH_ENTRY_IN_LL (g_filesystems_ll, FileSystem*, fs)
    {
        if (fs->driver == *driver_hit)
            return -EBUSY;
    }

    /* If we got here the FileSystemDriver is valid, and it's not
    in use by any mounted filesystems, so we remove it. */
    for (const FileSystemDriver** d = driver_hit;
         d < g_fs_drivers + g_fs_driver_count - 1;
         ++d)
        *d = *(d + 1);

    // FIXME: maybe shrink this array once we hit a threshold

    --g_fs_driver_count;

    return 0;
}

int vfs_open(const char* _USERPTR path, int flags, mode_t mode, Process* proc)
{
    char safe_path[PATH_MAX];
    int st = user_copy_str_from(path, safe_path, PATH_MAX);
    if (st < 0)
        return st;

    /* We only allow absolute paths */
    if (safe_path[0] != '/')
        return -EINVAL;

    FileSystem* fs = vfs_topmost_fs_for_path(safe_path);
    ASSERT(fs);

    VirtualNode* vnode = fs_lookup_vnode(safe_path, fs);
    if (!vnode)
    {
        /* Try to create it */
        if (flags & O_CREAT)
        {
            // FIXME: uid/gid
            ERR_OR(ino_t) tmp = fs_mkfile(safe_path, mode, 0, 0, fs);
            if (tmp.error < 0)
                return tmp.error;

            /* Try again, this time it should work */
            vnode = fs_lookup_vnode(safe_path, fs);
            ASSERT(vnode);
        }

        return -ENOENT;
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
    sysfd->vnode = vnode;

    return sysfd->fd;
}

ssize_t vfs_read(fd_t fd, void* _USERPTR buf, size_t n, Process* proc)
{
    FileDescriptor* sysfd = vfs_get_sysfd(fd, proc);
    if (!sysfd)
        return -EBADF;

    if (sysfd->vnode->mode & S_IFDIR)
        return -EISDIR;

    if (!(sysfd->flags & O_RDONLY))
        return -EBADF;

    if (!n)
        return 0;

    ssize_t rd = sysfd->vnode->ops->read(sysfd->vnode, buf, n, sysfd->off);
    if (rd < 0)
        return rd;

    sysfd->off += rd;
    return rd;
}

ssize_t vfs_write(fd_t fd, const void* _USERPTR buf, size_t n, Process* proc)
{
    FileDescriptor* sysfd = vfs_get_sysfd(fd, proc);
    if (!sysfd)
        return -EBADF;

    if (sysfd->vnode->mode & S_IFDIR)
        return -EISDIR;

    if (!BW_MASK(sysfd->flags, O_WRONLY))
        return -EBADF;

    /* Set offset to the end of the file if appending */
    if (sysfd->flags & O_APPEND)
        TODO_FATAL();

    if (!n)
        return 0;

    ssize_t wr = sysfd->vnode->ops->write(sysfd->vnode, buf, n, sysfd->off);
    if (wr < 0)
        return wr;

    sysfd->off += wr;
    return wr;
}

off_t vfs_lseek(fd_t fd, off_t off, int whence, Process* proc)
{
    FileDescriptor* sysfd = vfs_get_sysfd(fd, proc);
    if (!sysfd)
        return -EBADF;

    if (whence == SEEK_SET)
    {
        if (off < 0)
            return -EINVAL;

        sysfd->off = off;
    }
    else if (whence == SEEK_CUR)
    {
        off_t res = sysfd->off + off;
        if (res < 0)
            return off < 0 ? -EINVAL : -EOVERFLOW;

        sysfd->off = res;
    }
    else if (whence == SEEK_END)
    {
        off_t res = sysfd->vnode->size + off;
        if (res < 0)
            return off < 0 ? -EINVAL : -EOVERFLOW;

        sysfd->off = res;
    }
    else
    {
        return -EINVAL;
    }

    return sysfd->off;
}

int vfs_close(fd_t fd, Process* proc)
{
    size_t idx = proc_free_fd(fd, proc);
    if (idx == PLATFORM_MAX_UNSIGNED)
        return -EINVAL;

    ASSERT(idx < g_sys_fd_count);

    return vfs_free_file_descriptor(&g_sys_fds[idx]);
}

int sys_open(const char* path, int flags, mode_t mode)
{
    return vfs_open(path, flags, mode, sched_current_proc());
}

ssize_t sys_read(int fd, void* buf, size_t n)
{
    return vfs_read(fd, buf, n, sched_current_proc());
}
