/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_BLKDEV_H
#define _DXGMX_STORAGE_BLKDEV_H

/*
    A storage device is the base for all other storage devices eg: ATA.
    This is different from a storage drive which is the VFS abstraction over
    a storage device.
*/

#include <dxgmx/storage/types.h>
#include <dxgmx/types.h>

typedef struct S_BlockDevice
{
    char* name;
    char* type;

    u16 physical_sectorsize;

    u64 uid;

    /* Offset to apply to this block device when reading from it. */
    lba_t offset;
    /* How many sectors this block device has. */
    sectorcnt_t sector_count;

    ssize_t (*read)(
        const struct S_BlockDevice* self, lba_t lba, sectorcnt_t n, void* dest);
    ssize_t (*write)(
        const struct S_BlockDevice* self,
        lba_t lba,
        sectorcnt_t n,
        const void* src);

    /* Set if this is a partition. Is this really necessary ? */
    const struct S_BlockDevice* parent;

    /* Points to extra info created by the device driver. */
    void* extra;
} BlockDevice;

#endif // !_DXGMX_STORAGE_BLKDEV_H
