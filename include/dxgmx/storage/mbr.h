/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_MBR_H
#define _DXGMX_STORAGE_MBR_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_MBRPartition
{
    u8 drive_attrs;
    u8 chs_head_start;
    u8 chs_start_sector : 6;
    u16 chs_start_cylinder : 10;
    u8 system_id;
    u8 chs_ending_head;
    u8 chs_ending_sector : 6;
    u16 chs_ending_cylinder : 10;
    u32 lba_start;
    u32 sector_count;
} MBRPartition;

typedef struct _ATTR_PACKED S_MBR
{
    u8 bootstrap[440];
    u32 uid;
    u16 reserved;
    MBRPartition partitions[4];
    u16 signature;
} MBR;

/**
 * Read the MBR of a disk.
 *
 * 'dev' The block device to read from.
 * 'mbr' Destination mbr.
 *
 * Returns:
 * 0 on success.
 * -ENOMEM on out of memory.
 * other errno numbers come from the block device.
 */
int mbr_read(const BlockDevice* dev, MBR* mbr);

int mbr_validate(MBR* mbr);

int mbr_uuid_for_disk(const MBR* mbr, char* dest);
int mbr_uuid_for_part(const MBR* mbr, size_t part_idx, char* dest);

#endif // !_DXGMX_STORAGE_MBR_H
