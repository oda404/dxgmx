/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/stdio.h>
#include <dxgmx/storage/blkdevm.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/storage/partition.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/linkedlist.h>
#include <dxgmx/utils/uuid.h>

#define KLOGF_PREFIX "blkdevm: "

static LinkedList g_blkdev_drivers;
static LinkedList g_mountable_blkdevs;

static MountableBlockDevice* blkdevm_new_mountable_blkdev()
{
    MountableBlockDevice* mblkdev = kcalloc(sizeof(MountableBlockDevice));
    if (!mblkdev)
        return NULL;

    if (linkedlist_add(mblkdev, &g_mountable_blkdevs) < 0)
    {
        kfree(mblkdev);
        return NULL;
    }

    return mblkdev;
}

static void blkdevm_free_mountable_blkdev(MountableBlockDevice* mblkdev)
{
    linkedlist_remove_by_data(mblkdev, &g_mountable_blkdevs);
    kfree(mblkdev);
}

/**
 * Free all children of 'blkdev'.
 */
static void blkdevm_free_blkdev_children(BlockDevice* blkdev)
{
    FOR_EACH_ENTRY_IN_LL (g_mountable_blkdevs, MountableBlockDevice*, mblkdev)
    {
        /* FIXME: There is a subtle use after kfree type meme here, when calling
         * linkedlist_remove_by_data and looping in the for loop but it's fine
         * for now, since we are single-threaded non preemptible */
        if (mblkdev->parent == blkdev)
        {
            kfree(mblkdev->suffix);
            blkdevm_free_mountable_blkdev(mblkdev);
        }
    }
}

int blkdevm_register_blkdev_driver(BlockDeviceDriver* drv)
{
    int st = linkedlist_add(drv, &g_blkdev_drivers);
    if (st < 0)
        return st;

    if (drv->init)
    {
        st = drv->init(drv);
        if (st < 0)
        {
            linkedlist_remove_by_data(drv, &g_blkdev_drivers);
            return st;
        }
    }

    return 0;
}

int blkdevm_unregister_blkdev_driver(BlockDeviceDriver* drv)
{
    int st = linkedlist_remove_by_data(drv, &g_blkdev_drivers);
    if (st < 0)
        return st;

    if (drv->destroy)
        return drv->destroy(drv);

    return 0;
}

/**
 * Enumerate partitons on a block device.
 */
int blkdevm_enumerate_partitions(BlockDevice* dev)
{
    PartitionTable pt = {0};
    int st = partitiontable_create(dev, &pt);
    if (st < 0)
        return st;

    /* Remove any existing children (partitions) */
    blkdevm_free_blkdev_children(dev);

    for (size_t i = 0; i < pt.partition_count; ++i)
    {
        Partition* part = &pt.partitions[i];
        MountableBlockDevice mblkdev = {
            .offset = part->lba,
            .sector_count = part->sector_count,
            .sectorsize = dev->sectorsize,
            .read = partition_generic_read,
            .write = partition_generic_write,
            .parent = dev};

        mblkdev.suffix = strdup(part->suffix);
        if (!mblkdev.suffix)
        {
            partitiontable_destroy(&pt);
            return -ENOMEM;
        }

        MountableBlockDevice* new_mblkdev = blkdevm_new_mountable_blkdev();
        if (!new_mblkdev)
        {
            kfree(mblkdev.suffix);
            partitiontable_destroy(&pt);
            return -ENOMEM;
        }

        *new_mblkdev = mblkdev;
    }

    /* If the above loop fails allocating something and one or more partitions
     * have already been added, we will leave those be. */

    partitiontable_destroy(&pt);
    return 0;
}

const BlockDevice* blkdevm_find_raw_blkdev(const char* id)
{
    if (strstr(id, "UUID=") == id)
    {
        const char* rawuuid = id + sizeof("UUID=");
        return blkdevm_find_raw_blkdev_by_uuid(rawuuid);
    }
    else
    {
        return blkdevm_find_raw_blkdev_by_name(id);
    }
}

const BlockDevice* blkdevm_find_raw_blkdev_by_name(const char* name)
{
    FOR_EACH_ENTRY_IN_LL (g_blkdev_drivers, BlockDeviceDriver*, drv)
    {
        FOR_EACH_DRIVER_BLKDEV (drv, blkdev)
        {
            if (blkdev->name && strcmp(blkdev->name, name) == 0)
                return blkdev;
        }
    }

    return NULL;
}

const BlockDevice* blkdevm_find_raw_blkdev_by_uuid(const char* uuid)
{
    FOR_EACH_ENTRY_IN_LL (g_blkdev_drivers, BlockDeviceDriver*, drv)
    {
        FOR_EACH_DRIVER_BLKDEV (drv, blkdev)
        {
            if (blkdev->uuid && strcmp(blkdev->uuid, uuid) == 0)
                return blkdev;
        }
    }

    return NULL;
}

const MountableBlockDevice* blkdevm_find_mountable_blkdev(const char* id)
{
    if (strstr(id, "UUID=") == id)
    {
        const char* rawuuid = id + sizeof("UUID=");
        return blkdevm_find_mountable_blkdev_by_uuid(rawuuid);
    }
    else
    {
        return blkdevm_find_mountable_blkdev_by_name(id);
    }
}

const MountableBlockDevice*
blkdevm_find_mountable_blkdev_by_uuid(const char* uuid)
{
    FOR_EACH_ENTRY_IN_LL (g_mountable_blkdevs, MountableBlockDevice*, mblkdev)
    {
        if (strcmp(mblkdev->uuid, uuid) == 0)
            return mblkdev;
    }

    return NULL;
}

const MountableBlockDevice*
blkdevm_find_mountable_blkdev_by_name(const char* name)
{
#define MAX_BLKDEV_NAME_LEN 128

    char buf[MAX_BLKDEV_NAME_LEN + 1];
    FOR_EACH_ENTRY_IN_LL (g_mountable_blkdevs, MountableBlockDevice*, mblkdev)
    {
        size_t size = strlen(mblkdev->parent->name);
        if (size > MAX_BLKDEV_NAME_LEN)
            continue;

        strncpy(buf, mblkdev->parent->name, MAX_BLKDEV_NAME_LEN);
        strncat(buf, mblkdev->suffix, MAX_BLKDEV_NAME_LEN - size);

        if (strcmp(buf, name) == 0)
            return mblkdev;
    }

    return NULL;
}
