/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "ramfs.h"
#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/module.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>

#define KLOGF_PREFIX "ramfs: "

static void ramfs_destroy(FileSystem* fs)
{
    if (!fs)
        return;

    // if (fs->vnodes)
    // {
    //     FOR_EACH_ELEM_IN_DARR (fs->vnodes, fs->vnode_count, vnode)
    //     {
    //         if (vnode->name)
    //             kfree(vnode->name);
    //     }

    //     kfree(fs->vnodes);
    //     fs->vnodes = NULL;
    //     fs->vnode_count = 0;
    // }

    // if (fs->mntsrc)
    // {
    //     kfree(fs->mntsrc);
    //     fs->mntsrc = NULL;
    // }

    // RamFsMetadata* meta = fs->driver_ctx;

    // if (meta)
    // {
    //     if (meta->files)
    //     {
    //         FOR_EACH_ELEM_IN_DARR (meta->files, meta->files_count, file)
    //         {
    //             if (file->data)
    //                 kfree(file->data);
    //         }

    //         kfree(meta->files);
    //         meta->files = NULL;
    //         meta->files_count = 0;
    //     }

    //     kfree(meta);
    //     fs->driver_ctx = NULL;
    // }
}

static int
ramfs_init(const char* src, const char* type, const char* args, FileSystem* fs)
{
    (void)type;
    (void)args;

    if (!type || strcmp(type, "ramfs") != 0)
        return -EINVAL;

    RamFsMetadata* meta = kmalloc(sizeof(RamFsMetadata));
    if (!meta)
        return -ENOMEM;

    fs->driver_ctx = meta;
    fs->mntsrc = strdup(src == NULL ? "ramfs" : src);

    if (!fs->mntsrc)
    {
        ramfs_destroy(fs);
        return -ENOMEM;
    }

#define STARTING_FILES_COUNT 8

    meta->files = kcalloc(sizeof(RamFsFileData) * STARTING_FILES_COUNT);
    if (!meta->files)
    {
        ramfs_destroy(fs);
        return -ENOMEM;
    }

    meta->files_count = STARTING_FILES_COUNT;

#undef STARTING_FILES_COUNT

    return 0;
}

static int ramfs_mkfile(const char* path, mode_t mode, FileSystem* fs)
{
    if (!fs || !path)
        return -EINVAL;

    /* FIXME: we should validate the fs driver. */

    RamFsMetadata* meta = fs->driver_ctx;
    if (!meta)
        return -EINVAL;

    VirtualNode free_file_vnode;
    memset(&free_file_vnode, 0, sizeof(free_file_vnode));

    free_file_vnode.name = strdup(path);
    if (!free_file_vnode.name)
        return -ENOMEM;

    free_file_vnode.mode = mode;
    free_file_vnode.owner = fs;
    // FIXME: find the parent of the vnode.

    RamFsFileData* free_file = NULL;
    FOR_EACH_ELEM_IN_DARR (meta->files, meta->files_count, file)
    {
        ++free_file_vnode.n;
        if (!file->data)
        {
            free_file = file;
            break;
        }
    }

    if (!free_file)
    {
        /* we're out of free files */
        RamFsFileData* tmp = NULL;

        {
            tmp = krealloc(
                meta->files, sizeof(RamFsMetadata) * meta->files_count * 2);

            if (!tmp)
            {
                kfree(free_file_vnode.name);
                memset(free_file, 0, sizeof(RamFsFileData));
                return -ENOMEM;
            }

            meta->files = tmp;

            tmp = &meta->files[meta->files_count];
        }

        memset(tmp, 0, sizeof(RamFsFileData));

        /* Now the index should point to a new free file */
        free_file = tmp;
        free_file_vnode.n = meta->files_count;

        meta->files_count *= 2;
    }

    ASSERT(free_file);

    VirtualNode* tmpvnode = fs_new_vnode_cache(fs);
    if (!tmpvnode)
    {
        kfree(free_file_vnode.name);
        memset(free_file, 0, sizeof(RamFsFileData));
        return -ENOMEM;
    }

    tmpvnode->n = free_file_vnode.n;
    tmpvnode->mode = free_file_vnode.mode;
    tmpvnode->name = free_file_vnode.name;
    tmpvnode->size = free_file_vnode.size;
    tmpvnode->state = free_file_vnode.state;

    return 0;
}

static ssize_t
ramfs_read(const VirtualNode* vnode, void* buf, size_t n, loff_t off)
{
    if (!vnode || !buf)
        return -EINVAL;

    if (!n || off >= vnode->size)
        return 0;

    FileSystem* fs = vnode->owner;

    RamFsMetadata* meta = fs->driver_ctx;
    if (!meta || !meta->files)
        return -EINVAL;

    ASSERT(vnode->n > 0 && vnode->n < meta->files_count);

    RamFsFileData* filedata = &meta->files[vnode->n - 1];

    n = min(n, vnode->size - off);
    memcpy(buf, filedata->data + off, n);
    return n;
}

static ssize_t
ramfs_write(VirtualNode* vnode, const void* buf, size_t n, loff_t off)
{
    if (!vnode || !buf)
        return -EINVAL;

    if (!n)
        return 0;

    FileSystem* fs = vnode->owner;

    RamFsMetadata* meta = fs->driver_ctx;
    if (!meta || !meta->files)
        return -EINVAL;

    ASSERT(vnode->n > 0 && vnode->n < meta->files_count);

    RamFsFileData* filedata = &meta->files[vnode->n - 1];

    /* FIXME: check for overflow ? */
    const size_t write_size = off + n;
    if (write_size > vnode->size)
    {
        void* newdata = krealloc(filedata->data, write_size);
        if (!newdata)
            return -ENOMEM;

        filedata->data = newdata;

        if (off > vnode->size)
            memset(filedata->data + vnode->size, 0, off - vnode->size);

        vnode->size = write_size;
    }

    memcpy(filedata->data + off, buf, n);

    return n;
}

static int ramfs_mkdir(const char*, mode_t, FileSystem*)
{
    TODO_FATAL();
}

static VirtualNode* ramfs_vnode_lookup(const char* path, FileSystem* fs)
{
    /* Ramfs caches all it's vnodes */
    (void)path;
    (void)fs;
    return NULL;
}

#define MODULE_NAME "ramfs"

static const VirtualNodeOperations g_ramfs_vnode_ops = {
    .read = ramfs_read,
    .write = ramfs_write,
    .mkfile = ramfs_mkfile,
    .mkdir = ramfs_mkdir};

static const FileSystemDriver g_ramfs_driver = {
    .name = MODULE_NAME,
    .init = ramfs_init,
    .destroy = ramfs_destroy,
    .vnode_lookup = ramfs_vnode_lookup,
    .vnode_ops = &g_ramfs_vnode_ops};

static int ramfs_main()
{
    return vfs_register_fs_driver(&g_ramfs_driver);
}

static int ramfs_exit()
{
    return vfs_unregister_fs_driver(&g_ramfs_driver);
}

static MODULE const Module g_ramfs_module = {
    .name = MODULE_NAME, .main = ramfs_main, .exit = ramfs_exit};

#undef MODULE_NAME
