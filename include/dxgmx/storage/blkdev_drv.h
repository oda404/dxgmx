/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_BLKDEV_DRIVER_H
#define _DXGMX_STORAGE_BLKDEV_DRIVER_H

#include <dxgmx/storage/blkdev.h>
#include <dxgmx/utils/linkedlist.h>

typedef struct S_BlockDeviceDriver
{
    int (*init)(struct S_BlockDeviceDriver* drv);
    int (*destroy)(struct S_BlockDeviceDriver* drv);

    LinkedList blkdevs;
} BlockDeviceDriver;

/**
 * Create a new block device for the given driver.
 *
 * 'drv' The target driver. Non-NULL.
 *
 * Returns:
 * The address of the new block device on success.
 * NULL on out of memory.
 */
BlockDevice* blkdevdrv_new_blkdev(BlockDeviceDriver* drv);

/**
 * Free a block device that was registered on by a block device driver.
 *
 * No NULL pointers should be passed to this function.
 *
 * 'blkdev' The block device to remove.
 * 'drv' The target driver
 *
 * Returns:
 * 0 on success.
 * -ENOENT if 'blkdev' does not exist.
 */
int blkdevdrv_free_blkdev(BlockDevice* blkdev, BlockDeviceDriver* drv);

#define FOR_EACH_DRIVER_BLKDEV(_drv, _blkdev)                                  \
    FOR_EACH_ENTRY_IN_LL (_drv->blkdevs, BlockDevice*, _blkdev)

#endif // !_DXGMX_STORAGE_BLKDEV_DRIVER_Hs
