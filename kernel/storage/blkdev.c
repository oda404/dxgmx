/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kmalloc.h>
#include <dxgmx/storage/blkdev.h>

BlockDevice* blkdevlist_new(BlockDeviceList* blkdevlist)
{
    BlockDevice* dev = kcalloc(sizeof(BlockDevice));
    if (!dev)
        return NULL;

    if (linkedlist_add(dev, blkdevlist) < 0)
    {
        kfree(dev);
        return NULL;
    }

    return dev;
}

int blkdevlist_rm(BlockDevice* blkdev, BlockDeviceList* blkdevlist)
{
    kfree(blkdev);
    return linkedlist_remove_by_data(blkdev, blkdevlist);
}

size_t blkdevlist_size(const BlockDeviceList* blkdevlist)
{
    size_t count = 0;

    /* Maybe make the linked list cache it's size ? */
    FOR_EACH_ENTRY_IN_LL ((*blkdevlist), BlockDevice*, blkdev)
        ++count;

    return count;
}
