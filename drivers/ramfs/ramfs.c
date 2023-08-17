/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/module.h>
#include <dxgmx/posix/sys/stat.h>
#include <dxgmx/ramfs.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/user.h>

#define KLOGF_PREFIX "ramfs: "
#define RAMFS_DIR_MODE                                                         \
    (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH | S_IFDIR)

static int ramfs_enlarge_files(size_t newcount, RamFsMetadata* meta)
{
    RamFsFileData* tmp =
        krealloc(meta->files, sizeof(RamFsFileData) * newcount);
    if (!tmp)
        return -ENOMEM;

    meta->files = tmp;

    memset(
        meta->files + meta->file_capacity,
        0,
        (newcount - meta->file_capacity) * sizeof(RamFsFileData));

    meta->file_capacity = newcount;
    return 0;
}

static ERR_OR(fd_t) ramfs_find_next_avail_fd(RamFsMetadata* meta)
{
    while (meta->file_cursor < meta->file_capacity)
    {
        RamFsFileData* data = &meta->files[meta->file_cursor++];
        if (!data->used)
        {
            data->used = true;
            return VALUE(fd_t, meta->file_cursor);
        }
    }

    if (ramfs_enlarge_files(meta->file_capacity * 2, meta) < 0)
        return ERR(fd_t, -ENOMEM);

    return VALUE(fd_t, meta->file_cursor);
}

int ramfs_init(FileSystem* fs)
{
    RamFsMetadata* meta = kmalloc(sizeof(RamFsMetadata));
    if (!meta)
        return -ENOMEM;

    fs->driver_ctx = meta;

#define STARTING_FILES_COUNT 8
    meta->file_cursor = 0;
    if (ramfs_enlarge_files(STARTING_FILES_COUNT, meta) < 0)
    {
        ramfs_destroy(fs);
        return -ENOMEM;
    }
#undef STARTING_FILES_COUNT

    ERR_OR(ino_t) res = ramfs_mkfile(NULL, "/", RAMFS_DIR_MODE, 0, 0, fs);
    if (res.error < 0)
    {
        ramfs_destroy(fs);
        return res.error;
    }

    return 0;
}

void ramfs_destroy(FileSystem* fs)
{
    RamFsMetadata* meta = fs->driver_ctx;
    for (size_t i = 0; i < meta->file_capacity; ++i)
    {
        RamFsFileData* filedata = &meta->files[i];
        if (filedata->data)
        {
            kfree(filedata->data);
            filedata->data = NULL;
        }

        filedata->used = false;
    }

    if (meta->files)
    {
        kfree(meta->files);
        meta->files = NULL;
        meta->file_capacity = 0;
        meta->file_cursor = 0;
    }

    kfree(fs->driver_ctx);
    fs->driver_ctx = NULL;

    fs_free_all_cached_vnodes(fs);
}

ERR_OR(ino_t)
ramfs_mkfile(
    VirtualNode* dir,
    const char* name,
    mode_t mode,
    uid_t uid,
    gid_t gid,
    FileSystem* fs)
{
    RamFsMetadata* meta = fs->driver_ctx;
    ASSERT(meta);

    VirtualNode* vnode = fs_new_vnode_cache(name, fs);
    if (!vnode)
        return ERR(ino_t, -ENOMEM);

    ERR_OR(fd_t) fd_res = ramfs_find_next_avail_fd(meta);
    if (fd_res.error)
    {
        fs_free_cached_vnode(vnode, fs);
        return ERR(ino_t, fd_res.error);
    }

    vnode->n = fd_res.value;
    vnode->mode = mode;
    vnode->size = 0;
    vnode->uid = uid;
    vnode->gid = gid;
    vnode->parent = dir;

    RamFsFileData* filedata = &meta->files[vnode->n - 1];
    filedata->data = NULL;
    filedata->used = true;

    return VALUE(ino_t, vnode->n);
}

int ramfs_rmnode(VirtualNode* vnode)
{
    FileSystem* fs = vnode->owner;
    RamFsMetadata* meta = fs->driver_ctx;
    ASSERT(meta);

    if (vnode->mode & S_IFDIR)
    {
        TODO_FATAL();
    }
    else
    {
        meta->file_cursor = vnode->n - 1;

        RamFsFileData* filedata = &meta->files[vnode->n - 1];
        if (filedata->data)
        {
            kfree(filedata->data);
            filedata->data = NULL;
        }
        filedata->used = false;
    }

    return fs_free_cached_vnode(vnode, fs);
}

ssize_t
ramfs_read(const VirtualNode* vnode, _USERPTR void* buf, size_t n, off_t off)
{
    if (!n || (size_t)off >= vnode->size)
        return 0;

    FileSystem* fs = vnode->owner;
    RamFsMetadata* meta = fs->driver_ctx;
    ASSERT(meta);

    /* Cap n with respect to offset */
    n = min(n, vnode->size - off);
    RamFsFileData* filedata = &meta->files[vnode->n - 1];
    user_copy_to(buf, filedata->data + off, n);
    return n;
}

int ramfs_ioctl(VirtualNode*, int, void*)
{
    return -1;
}

ssize_t ramfs_write(VirtualNode*, const void*, size_t, off_t)
{
    return 0;
}

#define MODULE_NAME "ramfs"

static const VirtualNodeOperations g_ramfs_vnode_ops = {
    .read = ramfs_read, .write = ramfs_write, .ioctl = ramfs_ioctl};

static const FileSystemDriver g_ramfs_driver = {
    .name = MODULE_NAME,
    .generic_probe = false,
    .init = ramfs_init,
    .destroy = ramfs_destroy,
    .mkfile = ramfs_mkfile,
    .rmnode = ramfs_rmnode,
    .vnode_ops = &g_ramfs_vnode_ops};

static int ramfs_main()
{
    return vfs_register_fs_driver(&g_ramfs_driver);
}

static int ramfs_exit()
{
    return vfs_unregister_fs_driver(&g_ramfs_driver);
}

MODULE g_ramfs_module = {
    .name = MODULE_NAME,
    .main = ramfs_main,
    .stage = MODULE_STAGE3,
    .exit = ramfs_exit};

#undef MODULE_NAME
