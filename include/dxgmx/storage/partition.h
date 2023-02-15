/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_PARTITION_H
#define _DXGMX_STORAGE_PARTITION_H

#include <dxgmx/storage/blkdev.h>

ssize_t partition_generic_read(
    const BlockDevice* blkdev, lba_t lba, sectorcnt_t n, void* dest);

ssize_t partition_generic_write(
    const BlockDevice* blkdev, lba_t lba, sectorcnt_t n, const void* src);

/* Generic partition struct. */
typedef struct S_Partition
{
    lba_t lba;
    sectorcnt_t sector_count;
    char* suffix;
    char* uuid;
} Partition;

/* Generic partition table struct. */
typedef struct S_PartitionTable
{
    char* type;
    Partition* partitions;
    size_t partition_count;
} PartitionTable;

/**
 * Read and parse the partition table for a given block device. Once done with
 * the PartitionTable 'pt', it should bee freed using partitiontable_destroy.
 *
 * 'blkdev' The block device.
 * 'pt' The target partition table.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory.
 * -EINVAL if the block device did not contain any (known) partition tables.
 */
int partitiontable_create(const BlockDevice* blkdev, PartitionTable* pt);

/**
 * Free a partition table that was created with partitiontable_create.
 *
 * 'pt' The partition table.
 *
 * Returns:
 * 0 on success.
 */
int partitiontable_destroy(PartitionTable* pt);

#endif // !_DXGMX_STORAGE_PARTITION_H
