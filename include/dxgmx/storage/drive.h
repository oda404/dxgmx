/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_DRIVE_H
#define _DXGMX_STORAGE_DRIVE_H

#include <dxgmx/types.h>

struct S_GenericDrive;

/* Helper function to access a drive's data in an absolute manner. */
typedef bool (*storage_drive_read)(
    u64 start, size_t n, void* buf, const void* dev);

typedef bool (*storage_drive_write)(
    u64 start, size_t n, const void* buf, const void* dev);

struct S_GenericDrivePartition;

/* Helper functions to access a partition's data in a relative manner. */
typedef bool (*storage_drive_part_read)(
    u64 relstart,
    size_t n,
    void* buf,
    const struct S_GenericDrivePartition* part);

typedef bool (*storage_drive_part_write)(
    u64 relstart,
    size_t n,
    const void* buf,
    const struct S_GenericDrivePartition* part);

typedef struct S_GenericDrivePartition
{
    /* Suffix to be added at the end of the parent_drive->name eg: 'p0' */
    char* suffix;
    char* mountpoint;
    /* The filesystem type eg: fat, ext2 */
    char* fstype;
    /* Arbitrary number to index the partition. */
    size_t number;
    /* The LBA start of the partition */
    u64 lba_start;
    /* The number of sectors on the partition. */
    u64 sectors_count;

    storage_drive_part_read read;
    storage_drive_part_write write;

    const struct S_GenericDrive* parent_drive;
} GenericDrivePartition;

typedef struct S_GenericDrive
{
    /* The name of the device eg: hda */
    char* name;

    size_t physical_sectorsize;
    size_t logical_sectorsize;

    /* Unique IDentifier */
    u64 uid;

    GenericDrivePartition* partitions;
    size_t partitions_count;

    void* const internal_dev;
    const storage_drive_read read;
    const storage_drive_write write;
} GenericDrive;

#endif // !_DXGMX_STORAGE_DRIVE_H
