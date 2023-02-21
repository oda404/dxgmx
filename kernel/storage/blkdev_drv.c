/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kmalloc.h>
#include <dxgmx/storage/blkdev_drv.h>

BlockDevice* blkdevdrv_new_blkdev(BlockDeviceDriver* drv)
{
    BlockDevice* dev = kcalloc(sizeof(BlockDevice));
    if (!dev)
        return NULL;

    if (linkedlist_add(dev, &drv->blkdevs) < 0)
    {
        kfree(dev);
        return NULL;
    }

    return dev;
}

int blkdevdrv_free_blkdev(BlockDevice* blkdev, BlockDeviceDriver* drv)
{
    kfree(blkdev);
    return linkedlist_remove_by_data(blkdev, &drv->blkdevs);
}
