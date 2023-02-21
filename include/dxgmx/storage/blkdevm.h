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
#include <dxgmx/storage/blkdev_drv.h>
#include <dxgmx/types.h>

/** Register a block device driver with the block device manager. This function
 * will call the driver's init.
 *
 * 'drv' Non NULL driver pointer.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory
 * Other errnos come from the driver's destroy
 */
int blkdevm_register_blkdev_driver(BlockDeviceDriver* drv);

/** Unregister a block device driver from the block device manager. This
 * function will call the driver's destroy.
 *
 * 'drv' Non NULL driver pointer.
 *
 * Returns:
 * 0 on success.
 * -ENOENT if there is no such drv.
 * Other errnos come from the driver's destroy.
 */
int blkdevm_unregister_blkdev_driver(BlockDeviceDriver* drv);

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

/**
 * Find a registered block device by 'id'. The id can be a path to a block
 * device file on the filesystem like /dev/hda, a name like hda, or an UUID in
 * the form of UUID=... .
 *
 * 'id' Non NULL id string.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL if there is no block device identified by 'id'.
 */
const BlockDevice* blkdevm_find_raw_blkdev(const char* id);

/**
 * Find a registered block device by 'name'. The name is given by the driver to
 * a block device, for examples: hdX for pata.
 *
 * 'name' Non NULL name string.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL if there is no block device with 'name'.
 *
 */
const BlockDevice* blkdevm_find_raw_blkdev_by_name(const char* name);

/**
 * Find a registered block device by 'uuid'.
 *
 * Non NULL uuid string.
 *
 * Returns:
 * A BlockDevice* on success.
 * NULL if there is no block device with 'uuid'.
 */
const BlockDevice* blkdevm_find_raw_blkdev_by_uuid(const char* uuid);

/**
 * Find a registered mountable block device by 'id'. The id can be a path to a
 * mountable block device file on the filesystem like /dev/hdap0, a name like
 * hdap0, or an UUID in the form of UUID=... .
 *
 * 'id' Non NULL id string.
 *
 * Returns:
 * A MountableBlockDevice* on success.
 * NULL if there is no mountable block device identified by 'id'.
 */
const MountableBlockDevice* blkdevm_find_mountable_blkdev(const char* id);

/**
 * Find a registered mountable block device by 'uuid'.
 *
 * 'uuid' Non NULL uuid string.
 *
 * Returns:
 * A MountableBlockDevice* on success.
 * NULL if there is no mountable block device with 'uuid'.
 */
const MountableBlockDevice*
blkdevm_find_mountable_blkdev_by_uuid(const char* uuid);

/**
 * Find a registered mountable block device by 'name'. The name is given by the
 * driver to a block device, for examples: hdXpY for pata.
 *
 * 'name' Non NULL name string.
 *
 * Returns:
 * A MountableBlockDevice* on success.
 * NULL if there is no mountable block device with 'name'.
 *
 */
const MountableBlockDevice*
blkdevm_find_mountable_blkdev_by_name(const char* name);

#endif // !_DXGMX_STORAGE_BLKDEVM_H
