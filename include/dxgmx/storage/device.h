/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_DEVICE_H
#define _DXGMX_STORAGE_DEVICE_H

/*
    A storage device is the base for all other storage devices eg: ATA.
    This is different from a storage drive which is the VFS abstraction over
    a storage device.
*/

#include <dxgmx/types.h>

typedef u64 lba_t;
typedef u64 sector_t;

struct S_GenericStorageDevice;

typedef bool (*storage_dev_read)(
    lba_t lba,
    sector_t sectors,
    void* buf,
    const struct S_GenericStorageDevice* dev);

typedef bool (*storage_dev_write)(
    lba_t lba,
    sector_t sectors,
    const void* buf,
    const struct S_GenericStorageDevice* dev);

typedef struct S_GenericStorageDevice
{
    /* The storage device eg: hda */
    char* name;
    char* type;

    u16 physical_sectorsize;
    u16 logical_sectorsize;

    sector_t sectors_count;

    storage_dev_read read;
    storage_dev_write write;

    /* Points to extra info created by the device driver. */
    void* extra;
} GenericStorageDevice;

#endif // !_DXGMX_STORAGE_DEVICE_H
