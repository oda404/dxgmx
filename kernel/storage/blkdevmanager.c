/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/stdio.h>
#include <dxgmx/storage/blkdevmanager.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/uuid.h>

/* This is just a list of BlockDevices that have been registered
by drivers. */
static BlockDevice** g_blkdevs = NULL;
static size_t g_blkdevs_count = 0;

/* These two functions are kinda hackish, but letting
the drivers handle the offset seems even more hackish. */
static ssize_t
part_read(const struct S_BlockDevice* dev, lba_t lba, sectorcnt_t n, void* dest)
{
    return dev->parent->read(dev, dev->offset + lba, n, dest);
}

static ssize_t part_write(
    const struct S_BlockDevice* dev, lba_t lba, sectorcnt_t n, const void* src)
{
    return dev->parent->write(dev, dev->offset + lba, n, src);
}

/* Unlike linux, all partitions are suffixed with 'p' and the number. */
static int blkdevmanager_gen_part_name(BlockDevice* dev)
{
    if (!dev)
        return -EINVAL;

    const BlockDevice* parent = dev->parent;
    if (!parent)
        return -EINVAL; // device is not a partition

    size_t part_count = 0;
    FOR_EACH_ELEM_IN_DARR (g_blkdevs, g_blkdevs_count, blk)
        part_count += *blk != dev && (*blk)->parent == parent;

    /* The name is capped at 9 for now */
    if (part_count > 9)
        return -ENAMETOOLONG;

    size_t namelen = strlen(parent->name) + 2; // hardcoded to 2 for now
    if (!(dev->name = kmalloc(namelen + 1)))
        return -ENOMEM;

    sprintf(dev->name, "%sp%d", parent->name, part_count);

    return 0;
}

static BlockDevice** blkdevmanager_new_blkdev()
{
    BlockDevice** tmp =
        krealloc(g_blkdevs, (g_blkdevs_count + 1) * sizeof(BlockDevice*));

    if (!tmp)
        return NULL;

    g_blkdevs = tmp;
    ++g_blkdevs_count;

    return &g_blkdevs[g_blkdevs_count - 1];
}

static int blkdevmanager_parse_bootsector(BlockDevice* dev)
{
    if (!dev)
        return -EINVAL;

    Mbr mbr;

    {
        const int st = mbr_read(dev, &mbr);
        if (st < 0)
            return st;
    }

    if (mbr.signature != 0xAA55)
        return -ENODEV;

    dev->uuid = kmalloc(UUID_LENGTH + 1);
    if (!dev->uuid)
        return -ENOMEM;

    mbr_uuid_for_disk(&mbr, dev->uuid);

    for (size_t i = 0; i < 4; ++i)
    {
        MbrPartition* mbrpart = &mbr.partitions[i];

        if (!mbrpart->lba_start || !mbrpart->sector_count)
            continue; // consider unallocated

        BlockDevice** part = blkdevmanager_new_blkdev();
        if (!part)
            return -ENOMEM;

        *part = kmalloc(sizeof(BlockDevice));
        if (!(*part))
            return -ENOMEM;

        /* The partition is the same as the parent device,
        with some minor changes. */
        **part = *dev;

        (*part)->name = NULL;

        (*part)->read = part_read;
        (*part)->write = part_write;
        (*part)->offset = mbrpart->lba_start;
        (*part)->sector_count = mbrpart->sector_count;
        (*part)->parent = dev;

        (*part)->uuid = kmalloc(UUID_LENGTH + 1);
        if (!(*part)->uuid)
        {
            blkdevmanager_unregister_dev(*part);
            return -ENOMEM;
        }

        mbr_uuid_for_part(&mbr, i, (*part)->uuid);

        blkdevmanager_gen_part_name(*part);
    }

    return 0;
}

bool blkdevmanager_init()
{
    // dang
    return true;
}

int blkdevmanager_register_dev(BlockDevice* dev)
{
    BlockDevice** newblkdev = blkdevmanager_new_blkdev();
    if (!newblkdev)
        return -ENOMEM;

    *newblkdev = dev;

    /* If the device doesn't have a parent assume it's a hard disk
    and it might have an mbr/gpt and therefore partitions :) */
    if (!dev->parent)
        blkdevmanager_parse_bootsector(dev);

    return 0;
}

int blkdevmanager_unregister_dev(BlockDevice* dev)
{
    for (size_t i = 0; i < g_blkdevs_count; ++i)
    {
        BlockDevice* blk = g_blkdevs[i];
        if (blk != dev)
            continue;

        if (blk->parent)
        {
            /* if the block blkice has a parent it means it
            has been generated by blkblkmanager and we need to do
            the cleanup */
            kfree(blk->name);
            blk->name = NULL;
            kfree(blk);
        }

        kfree(blk->uuid);
        blk->uuid = NULL;

        for (size_t k = i; k < g_blkdevs_count - 1; ++k)
            g_blkdevs[k] = g_blkdevs[k + 1];

        break;
    }

    return 0;
}

const BlockDevice* blkdevm_find_blkdev_by_id(const char* id)
{
    if (!id)
        return NULL;

    if (strstr(id, "UUID=") == id)
    {
        const char* rawuuid = id + sizeof("UUID=");
        return blkdevmanager_find_blkdev_by_uuid(rawuuid);
    }
    else
    {
        return blkdevmanager_find_blkdev_by_name(id);
    }
}

const BlockDevice* blkdevmanager_find_blkdev_by_name(const char* name)
{
    if (!name)
        return NULL;

    FOR_EACH_ELEM_IN_DARR (g_blkdevs, g_blkdevs_count, blk)
    {
        if ((*blk)->name && strcmp((*blk)->name, name) == 0)
            return *blk;
    }

    return NULL;
}

const BlockDevice* blkdevmanager_find_blkdev_by_uuid(const char* uuid)
{
    if (!uuid)
        return NULL;

    FOR_EACH_ELEM_IN_DARR (g_blkdevs, g_blkdevs_count, blk)
    {
        if ((*blk)->uuid && strcmp((*blk)->uuid, uuid) == 0)
            return *blk;
    }

    return NULL;
}
