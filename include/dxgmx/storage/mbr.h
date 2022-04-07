/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_MBR_H
#define _DXGMX_STORAGE_MBR_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_MBRPartitionTableEntry
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
} MBRPartitionTableEntry;

typedef struct _ATTR_PACKED S_MBR
{
    u8 bootstrap[440];
    u32 uid;
    u16 reserved;
    MBRPartitionTableEntry part1;
    MBRPartitionTableEntry part2;
    MBRPartitionTableEntry part3;
    MBRPartitionTableEntry part4;
    u16 signature;
} MBR;

int mbr_read(BlockDevice* dev, MBR* mbr_out);

#endif // !_DXGMX_STORAGE_MBR_H
