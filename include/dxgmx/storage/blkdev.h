/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_BLKDEV_H
#define _DXGMX_STORAGE_BLKDEV_H

/**
 * A BlockDevice is the generic API of a storage device. Every driver keeps
 * track of it's own BlockDevices, that it also registers with blkdevm.
 */

#include <dxgmx/storage/types.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/linkedlist.h>

struct S_BlockDevice;

typedef ssize_t (*blkdev_read_t)(
    const struct S_BlockDevice* blkdev, lba_t lba, sectorcnt_t n, void* dest);

typedef ssize_t (*blkdev_write_t)(
    const struct S_BlockDevice* blkdev,
    lba_t lba,
    sectorcnt_t n,
    const void* src);

typedef struct S_BlockDevice
{
    /* The name of the block device. (For PATA devices it will be hdX, for SATA
     * sdX) */
    char* name;

    /* The type of block device. (For example "pata" for PATA devices). */
    char* type;

    u16 physical_sectorsize;

    char* uuid;

    /* Offset to apply to this block device when reading from it. */
    lba_t offset;
    /* How many sectors this block device has. */
    sectorcnt_t sector_count;

    blkdev_read_t read;
    blkdev_write_t write;

    /* The parent of this BlockDevice (Really only set if the BlockDevice is a
     * partiton.) */
    const struct S_BlockDevice* parent;

    /* Points to extra info created by the device driver. */
    void* extra;
} BlockDevice;

/* Utilities used by drivers to keep track of their own block devices. */

typedef LinkedList BlockDeviceList;

#define FOR_EACH_BLKDEV(_blkdevlist, _blkdev)                                  \
    FOR_EACH_ENTRY_IN_LL (_blkdevlist, BlockDevice*, _blkdev)

/**
 * Create a new block device for the given list.
 *
 * 'bkldevlist' The block device list. Non-NULL.
 *
 * Returns:
 * The address of the new block device on success.
 * NULL on out of memory.
 */
BlockDevice* blkdevlist_new(BlockDeviceList* blkdevlist);

/**
 * Remove a block device from a block device list by value.
 *
 * No NULL pointers should be passed to this function.
 *
 * 'blkdev' The block device to remove.
 * 'blkdevlist' The block device list to remove from.
 *
 * Returns:
 * 0 on success.
 * -ENOENT if 'blkdev' is not part of 'blkdevlist'.
 */
int blkdevlist_rm(BlockDevice* blkdev, BlockDeviceList* blkdevlist);

/**
 * Get the number of block devices in a block device list.
 *
 * 'blkdevlist' The block device list.
 *
 * Returns:
 * The number of elements in the list.
 */
size_t blkdevlist_size(const BlockDeviceList* blkdevlist);

#endif // !_DXGMX_STORAGE_BLKDEV_H
