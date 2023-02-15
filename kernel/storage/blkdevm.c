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

static BlockDeviceList g_blkdevs;

/**
 * Free all children of 'blkdev'.
 *
 * Returns:
 * 0 on success.
 */
static int blkdevm_free_blkdev_children(BlockDevice* blkdev)
{
    FOR_EACH_ENTRY_IN_LL (g_blkdevs, BlockDevice*, child)
    {
        /* FIXME: There is a subtle use after kfree type meme here, when calling
         * linkedlist_remove_by_data, and looping in the for loop but it's fine
         * for now, since we are single-threaded non preemptible */
        if (child->parent == blkdev)
        {
            /* This was allocated by us. */
            kfree(child->name);

            linkedlist_remove_by_data(child, &g_blkdevs);
            kfree(child);
        }
    }

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
        BlockDevice partblkdev = *dev;

        /* Glue together the base device name and partition suffix */
        size_t partname_len = strlen(dev->name) + strlen(part->suffix);
        partblkdev.name = kmalloc(partname_len + 1);
        if (!partblkdev.name)
        {
            partitiontable_destroy(&pt);
            return -ENOMEM;
        }

        strcpy(partblkdev.name, dev->name);
        strcat(partblkdev.name, part->suffix);

        /* Set the other stuff */
        partblkdev.offset = part->lba;
        partblkdev.sector_count = part->sector_count;
        partblkdev.read = partition_generic_read;
        partblkdev.write = partition_generic_write;
        partblkdev.parent = dev;

        BlockDevice* newdev = blkdevm_new_blkdev();
        if (!newdev)
        {
            kfree(partblkdev.name);
            partitiontable_destroy(&pt);
            return -ENOMEM;
        }

        *newdev = partblkdev;
    }

    /* If the above loop fails allocating something and one or more partitions
     * have already been added, we will leave those be. */

    partitiontable_destroy(&pt);
    return 0;
}

BlockDevice* blkdevm_new_blkdev()
{
    BlockDevice* blkdev = kcalloc(sizeof(BlockDevice));
    if (!blkdev)
        return NULL;

    if (linkedlist_add(blkdev, &g_blkdevs) < 0)
        return NULL;

    return blkdev;
}

int blkdevm_free_blkdev(BlockDevice* blkdev)
{
    /* blkdevm doesn't allocate anything in the blkdev itself, only stuff to
     * hold the block device. */

    /* FIXME: we should check with the vfs, or at least let it know some block
     * device was removed. */

    /* We don't leave children without a parent. */
    blkdevm_free_blkdev_children(blkdev);
    kfree(blkdev);
    linkedlist_remove_by_data(blkdev, &g_blkdevs);
    return 0;
}

BlockDeviceList* blkdevm_get_blkdevs()
{
    return &g_blkdevs;
}

const BlockDevice* blkdevm_find_blkdev(const char* id)
{
    if (!id)
        return NULL;

    if (strstr(id, "UUID=") == id)
    {
        const char* rawuuid = id + sizeof("UUID=");
        return blkdevm_find_blkdev_by_uuid(rawuuid);
    }
    else
    {
        return blkdevm_find_blkdev_by_name(id);
    }
}

const BlockDevice* blkdevm_find_blkdev_by_name(const char* name)
{
    if (!name)
        return NULL;

    FOR_EACH_ENTRY_IN_LL (g_blkdevs, BlockDevice*, blkdev)
    {
        if (blkdev->name && strcmp(blkdev->name, name) == 0)
            return blkdev;
    }

    return NULL;
}

const BlockDevice* blkdevm_find_blkdev_by_uuid(const char* uuid)
{
    if (!uuid)
        return NULL;

    FOR_EACH_ENTRY_IN_LL (g_blkdevs, BlockDevice*, blkdev)
    {
        if (blkdev->uuid && strcmp(blkdev->uuid, uuid) == 0)
            return blkdev;
    }

    return NULL;
}
