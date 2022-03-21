/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_DRIVE_H
#define _DXGMX_STORAGE_DRIVE_H

#include <dxgmx/storage/device.h>
#include <dxgmx/types.h>

struct S_GenericDrivePartition;

/* Helper functions to access a partition's data in a relative manner. */
typedef bool (*storage_drive_part_read)(
    lba_t relstart,
    sector_t sectors,
    void* buf,
    const struct S_GenericDrivePartition* part);

typedef bool (*storage_drive_part_write)(
    lba_t relstart,
    sector_t sectors,
    const void* buf,
    const struct S_GenericDrivePartition* part);

typedef struct S_GenericDrivePartition
{
    /* Suffix to be added at the end of the parent_drive->name eg: 'p0' */
    char* suffix;
    char* mountpoint;
    /* Arbitrary number to index the partition. */
    size_t number;
    /* The LBA start of the partition */
    lba_t lba_start;
    /* The number of sectors on the partition. */
    sector_t sectors_count;

    storage_drive_part_read read;
    storage_drive_part_write write;

    const struct S_GenericDrive* parent_drive;

    /* The filesystem type eg: fat, ext2 */
    char* fstype;
    /* Extra information to be stored for the filesystem */
    void* extra;
} GenericDrivePartition;

typedef struct S_GenericDrive
{
    /* Unique IDentifier */
    u64 uid;

    GenericDrivePartition* partitions;
    size_t partitions_count;

    /* The underlying device. */
    const GenericStorageDevice* dev;
} GenericDrive;

#endif // !_DXGMX_STORAGE_DRIVE_H
