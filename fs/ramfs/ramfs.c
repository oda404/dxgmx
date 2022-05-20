/**
 * Copyright 2022 Alexandru Olaru.
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
#include <dxgmx/types.h>

#define KLOGF_PREFIX "ramfs: "

static int ramfs_init(FileSystem* fs)
{
    fs->driver_ctx = kmalloc(sizeof(RamFsMetadata));
    if (!fs->driver_ctx)
        return -ENOMEM;

    RamFsMetadata* meta = fs->driver_ctx;

#define STARTING_FILES_COUNT 8

    meta->files = kcalloc(sizeof(RamFsFileData) * STARTING_FILES_COUNT);
    if (!meta->files)
    {
        kfree(meta);
        return -ENOMEM;
    }

    meta->files_count = STARTING_FILES_COUNT;

#undef STARTING_FILES_COUNT

    return 0;
}

static void ramfs_destroy(FileSystem* fs)
{
    if (!fs)
        return;

    if (fs->vnodes)
    {
        FOR_EACH_ELEM_IN_DARR (fs->vnodes, fs->vnode_count, vnode)
        {
            if (vnode->name)
                kfree(vnode->name);
        }

        kfree(fs->vnodes);
        fs->vnodes = NULL;
        fs->vnode_count = 0;
    }

    RamFsMetadata* meta = fs->driver_ctx;

    if (meta)
    {
        if (meta->files)
        {
            FOR_EACH_ELEM_IN_DARR (meta->files, meta->files_count, file)
            {
                if (file->data)
                    kfree(file->data);
            }

            kfree(meta->files);
            meta->files = NULL;
            meta->files_count = 0;
        }

        kfree(meta);
        fs->driver_ctx = NULL;
    }
}

static int ramfs_mkfile(FileSystem* fs, const char* path, mode_t mode)
{
    if (!fs || !path)
    {
        errno = EINVAL;
        return -1;
    }

    /* FIXME: we should validate the fs driver. */

    RamFsMetadata* meta = fs->driver_ctx;
    if (!meta)
    {
        errno = EINVAL;
        return -1;
    }

    VirtualNode free_file_vnode;
    memset(&free_file_vnode, 0, sizeof(free_file_vnode));

    free_file_vnode.name = strdup(path);
    if (!free_file_vnode.name)
    {
        errno = ENOMEM;
        return -1;
    }

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
                errno = ENOMEM;
                goto fail;
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

    VirtualNode* tmpvnode = fs_new_vnode(fs);
    if (!tmpvnode)
    {
        errno = ENOMEM;
        goto fail;
    }

    *tmpvnode = free_file_vnode;

    return 0;

fail:
    if (free_file_vnode.name)
        kfree(free_file_vnode.name);

    if (free_file)
        memset(free_file, 0, sizeof(RamFsFileData));

    return -1;
}

static ssize_t ramfs_read(
    FileSystem* fs, const VirtualNode* vnode, void* buf, size_t n, loff_t off)
{
    if (!fs || !vnode || !buf || vnode->owner != fs)
    {
        errno = EINVAL;
        return -1;
    }

    if (!n || off >= vnode->size)
        return 0;

    RamFsMetadata* meta = fs->driver_ctx;
    if (!meta || !meta->files)
    {
        errno = EINVAL;
        return -1;
    }

    ASSERT(vnode->n > 0 && vnode->n < meta->files_count);

    RamFsFileData* filedata = &meta->files[vnode->n - 1];

    n = min(n, vnode->size - off);
    memcpy(buf, filedata->data + off, n);
    return n;
}

static ssize_t ramfs_write(
    FileSystem* fs, VirtualNode* vnode, const void* buf, size_t n, loff_t off)
{
    if (!fs || !vnode || !buf || vnode->owner != fs)
    {
        errno = EINVAL;
        return -1;
    }

    if (!n)
        return 0;

    RamFsMetadata* meta = fs->driver_ctx;
    if (!meta || !meta->files)
    {
        errno = EINVAL;
        return -1;
    }

    ASSERT(vnode->n > 0 && vnode->n < meta->files_count);

    RamFsFileData* filedata = &meta->files[vnode->n - 1];

    /* FIXME: check for overflow ? */
    const size_t write_size = off + n;
    if (write_size > vnode->size)
    {
        void* newdata = krealloc(filedata->data, write_size);
        if (!newdata)
        {
            errno = ENOMEM;
            return -1;
        }

        filedata->data = newdata;

        if (off > vnode->size)
            memset(filedata->data + vnode->size, 0, off - vnode->size);

        vnode->size = write_size;
    }

    memcpy(filedata->data + off, buf, n);

    return n;
}

#define MODULE_NAME "ramfs"

static int ramfs_main()
{
    const FileSystemDriver ramfs_driver = {
        .name = MODULE_NAME,
        .backing = FILESYSTEM_BACKING_RAM,
        .init = ramfs_init,
        .destroy = ramfs_destroy,
        .read = ramfs_read,
        .write = ramfs_write,
        .mkfile = ramfs_mkfile};

    return vfs_register_fs_driver(&ramfs_driver);
}

static int ramfs_exit()
{
    return vfs_unregister_fs_driver(MODULE_NAME);
}

static MODULE const Module g_ramfs_module = {
    .name = MODULE_NAME, .main = ramfs_main, .exit = ramfs_exit};

#undef MODULE_NAME
