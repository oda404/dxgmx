/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_MBR_H
#define _DXGMX_STORAGE_MBR_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_MbrPartition
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
} MbrPartition;

typedef struct _ATTR_PACKED S_Mbr
{
    u8 bootstrap[440];
    u32 uid;
    u16 reserved;
    MbrPartition partitions[4];
    u16 signature;
} Mbr;

int mbr_read(BlockDevice* dev, Mbr* mbr_out);
int mbr_uuid_for_disk(const Mbr* mbr, char* dest);
int mbr_uuid_for_part(const Mbr* mbr, size_t part_idx, char* dest);

#endif // !_DXGMX_STORAGE_MBR_H
