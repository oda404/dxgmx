/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_DRIVE_H
#define _DXGMX_STORAGE_DRIVE_H

#include <dxgmx/types.h>

typedef bool (*storage_drive_read)(
    u64 start, size_t n, u8* buf, const void* dev);
typedef bool (*storage_drive_write)(
    u64 start, size_t n, const u8* buf, const void* dev);

struct S_GenericDrive;

typedef struct S_GenericDrivePartition
{
    char* suffix;
    size_t suffixlen;

    char* mountpoint;
    size_t mountpoint_len;

    size_t number;

    u64 start;
    u64 size;

    const struct S_GenericDrive* parent_drive;
} GenericDrivePartition;

typedef struct S_GenericDrive
{
    /* The name of the device eg: hda */
    char* name;

    const size_t sectorsize;

    /* Unique IDentifier */
    u64 uid;

    GenericDrivePartition* partitions;
    size_t partitions_count;

    void* const internal_dev;
    const storage_drive_read read;
    const storage_drive_write write;
} GenericDrive;

#endif // !_DXGMX_STORAGE_DRIVE_H
