/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/stdlib.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/storage/partition.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/uuid.h>

/**
 * Generate a partition suffix based on n. All partitions suffixes in dxgmx are
 * in the format if 'p<number>'. The pointer returned by this function
 * needs to be kfreed once done with.
 *
 * 'n' The partition number;
 *
 * Returns:
 * A char* if successful.
 * NULL on out of memory.
 */
static char* partition_generate_suffix(size_t n)
{
    size_t dig_count = 0;

    /* Kinda stupid, but I don't want to use log10 thing. */
    size_t tmpn = n;
    while (tmpn)
    {
        ++dig_count;
        tmpn /= 10;
    }

    char* suffix = kcalloc(dig_count + 2); // p and the NULL terminator.
    if (!suffix)
        return NULL;

    suffix[0] = 'p';
    itoa(n, suffix + 1, 10);

    return suffix;
}

/**
 * Try to parse the MBR and fill out the given partition table.
 *
 * Returns:
 * 0 on success.
 * -EINVAL if there is no mbr present on the device.
 */
static int
partitiontable_try_create_mbr(const BlockDevice* dev, PartitionTable* pt)
{
    MBR mbr;
    int st = mbr_read(dev, &mbr);
    if (st < 0)
        return st;

    st = mbr_validate(&mbr);
    if (st < 0)
        return st;

    /* Count the partitions */
    size_t partition_count = 0;
    for (size_t i = 0; i < 4; ++i)
    {
        MBRPartition* mbrpart = &mbr.partitions[i];

        if (!mbrpart->lba_start || !mbrpart->sector_count)
            continue; // consider unallocated

        ++partition_count;
    }

    pt->partition_count = partition_count;
    pt->type = "mbr";

    /* If there are no partitions we're done */
    if (!partition_count)
        return 0;

    pt->partitions = kcalloc(sizeof(Partition) * partition_count);
    if (!pt->partitions)
        return -ENOMEM;

    size_t idx = 0;
    for (size_t i = 0; i < 4; ++i)
    {
        MBRPartition* mbrpart = &mbr.partitions[i];

        if (!mbrpart->lba_start || !mbrpart->sector_count)
            continue; // consider unallocated

        Partition* part = &pt->partitions[idx];

        part->suffix = partition_generate_suffix(idx);
        if (!part->suffix)
        {
            partitiontable_destroy(pt);
            return -ENOMEM;
        }

        part->uuid = kmalloc(UUID_LENGTH + 1);
        if (!part->uuid)
        {
            partitiontable_destroy(pt);
            return -ENOMEM;
        }

        mbr_uuid_for_part(&mbr, idx, part->uuid);

        part->lba = mbrpart->lba_start;
        part->sector_count = mbrpart->sector_count;

        ++idx;
    }

    return 0;
}

ssize_t partition_generic_read(
    const BlockDevice* blkdev, lba_t lba, sectorcnt_t n, void* dest)
{
    return blkdev->parent->read(blkdev, blkdev->offset + lba, n, dest);
}

ssize_t partition_generic_write(
    const BlockDevice* blkdev, lba_t lba, sectorcnt_t n, const void* src)
{
    return blkdev->parent->write(blkdev, blkdev->offset + lba, n, src);
}

int partitiontable_create(const BlockDevice* blkdev, PartitionTable* pt)
{
    return partitiontable_try_create_mbr(blkdev, pt);
}

int partitiontable_destroy(PartitionTable* pt)
{
    /* Free the suffix of each partition. */
    for (size_t i = 0; i < pt->partition_count; ++i)
    {
        Partition* part = &pt->partitions[i];

        if (part->suffix)
            kfree(part->suffix);

        if (part->uuid)
            kfree(part->uuid);
    }

    if (pt->partitions)
        kfree(pt->partitions);

    return 0;
}
