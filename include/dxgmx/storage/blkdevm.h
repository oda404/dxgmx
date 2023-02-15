/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_BLKDEVM_H
#define _DXGMX_STORAGE_BLKDEVM_H

/* Block device manager. I can think of a lot of shortcomings for this API but,
 * to be completely honest I don't really know how a driver/the OS will really
 * interact with it, so I don't know how to design it better. We'll just
 * add/remove from it as we go... */

#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

/**
 * Create a new block device. The returned block device is zeroed out, it is the
 * caller's to fill it out.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL on out of memory.
 */
BlockDevice* blkdevm_new_blkdev();

/**
 * Free a block device allocated by blkdevm_new_blkdev.
 *
 * 'blkdev' The block device to free.
 *
 * Returns:
 * 0 on success.
 */
int blkdevm_free_blkdev(BlockDevice* blkdev);

/**
 * Enumerate and register the partitions of a block device.
 *
 * 'dev' The block device.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory.
 * -EINVAL if no (known) partitions have been found.
 */
int blkdevm_enumerate_partitions(BlockDevice* dev);

BlockDeviceList* blkdevm_get_blkdevs();

/**
 * Find a registered block device by 'id'. The id can be a path to a block
 * device file on the filesystem like /dev/hdap0, a name like hdap0, or an UUID.
 *
 * 'id' Non NULL id string.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL if there is no block device identified by 'id'.
 */
const BlockDevice* blkdevm_find_blkdev(const char* id);

/**
 * Find a registered block device by 'name'. The name is given by the driver to
 * a block device, for examples: hdXpY for pata.
 *
 * 'name' Non NULL name string.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL if there is no block device with 'name'.
 *
 */
const BlockDevice* blkdevm_find_blkdev_by_name(const char* name);

/**
 * Find a registered block device by 'uuid'.
 *
 * Non NULL uuid string.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL if there is no block device with 'uuid'.
 */
const BlockDevice* blkdevm_find_blkdev_by_uuid(const char* uuid);

#endif // !_DXGMX_STORAGE_BLKDEVM_H
