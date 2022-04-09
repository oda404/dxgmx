/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/string.h>
#include <dxgmx/utils/uuid.h>

int mbr_read(BlockDevice* dev, Mbr* mbr_out)
{
    if (!dev || !mbr_out)
        return -EINVAL;

    if (!dev->read(dev, 0, 1, mbr_out))
        return -EIO;

    return 0;
}

int mbr_uuid_for_disk(const Mbr* mbr, char* dest)
{
    if (!mbr || !dest)
        return -EINVAL;

    u32 a = mbr->uid;
    u16 b = mbr->bootstrap[8] ^ mbr->bootstrap[9] ^ mbr->bootstrap[10] ^
            mbr->bootstrap[11];
    u16 c = mbr->bootstrap[0] ^ mbr->bootstrap[1] ^ mbr->bootstrap[2] ^
            mbr->bootstrap[3];
    u16 d = mbr->bootstrap[4] ^ mbr->bootstrap[5] ^ mbr->bootstrap[6] ^
            mbr->bootstrap[7];
    u64 e = 0;

    return uuid_format(a, b, c, d, e, dest);
}

int mbr_uuid_for_part(const Mbr* mbr, size_t part_idx, char* dest)
{
    if (!mbr || !dest || part_idx >= 4)
        return -EINVAL;

    const MbrPartition* mbrpart = &mbr->partitions[part_idx];

    u16 partchecksum = *((u16*)mbrpart);

    u32 a = mbr->uid;
    u16 b = partchecksum;
    u16 c = mbr->bootstrap[0] ^ mbr->bootstrap[1] ^ mbr->bootstrap[2] ^
            mbr->bootstrap[3];
    u16 d = mbr->bootstrap[4] ^ mbr->bootstrap[5] ^ mbr->bootstrap[6] ^
            mbr->bootstrap[7];
    u64 e = 0;

    return uuid_format(a, b, c, d, e, dest);
}
