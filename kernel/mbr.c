/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/storage/mbr.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/string.h>

bool mbr_drive_has_mbr(const GenericDrive *drive)
{
    if(!drive)
        return false;

    MBR *mbr = kmalloc(512);
    if(!mbr || !drive->read(0, 512, (void *)mbr, drive->internal_dev))
    {
        kfree(mbr);
        return false;
    }

    const bool ret = mbr->signature == 0xAA55;

    kfree(mbr);
    return ret;
}

bool mbr_parse_drive_info(GenericDrive *drive)
{
    if(!drive)
        return false;
    
    MBR *mbr = kmalloc(512);
    if(!mbr || !drive->read(0, 512, (void *)mbr, drive->internal_dev))
    {
        kfree(mbr);
        return false;
    }
    
    drive->uid = mbr->uid;
    
    const MBRPartitionTableEntry *mbrpart = &mbr->partition1;
    for(size_t i = 0; i < 4; ++i, ++mbrpart)
    {
        if(!mbrpart->lba_start || !mbrpart->total_sectors)
            continue; // consider unallocated.

        drive->partitions = krealloc(
            drive->partitions,
            sizeof(GenericDrivePartition) * (++drive->partitions_count)
        );

        if(!drive->partitions)
        {
            kfree(mbr);
            return false;
        }

        GenericDrivePartition *part = &drive->partitions[drive->partitions_count - 1];
        memset(part, 0, sizeof(GenericDrivePartition));

        part->start = mbrpart->lba_start * drive->sectorsize;
        part->size = mbrpart->total_sectors * drive->sectorsize;
        part->number = i;
        part->parent_drive = drive;
    }

    kfree(mbr);
    return true;
}
