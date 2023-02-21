/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/utils/uuid.h>

int mbr_read(const BlockDevice* dev, MBR* mbr)
{
    /* An MBR is 512 bytes, if we have a sector size of 512, we can skip
     * allocating a bigger buffer. */
    if (dev->sectorsize > 512)
    {
        u8* buf = kmalloc(dev->sectorsize);
        if (!buf)
            return -ENOMEM;

        int st = dev->read(dev, 0, 1, buf);
        if (st < 0)
        {
            kfree(buf);
            return st;
        }

        *mbr = *((MBR*)buf);
        kfree(buf);
    }
    else
    {
        int st = dev->read(dev, 0, 1, mbr);
        if (st < 0)
            return st;
    }

    return 0;
}

int mbr_validate(MBR* mbr)
{
    if (mbr->signature != 0xAA55)
        return -EINVAL;

    /* 0x5A5A if copy-protected, allegedly */
    if (mbr->reserved != 0 && mbr->signature != 0x5A5A)
        return -EINVAL;

    return 0;
}

int mbr_uuid_for_disk(const MBR* mbr, char* dest)
{
    return uuid_format(mbr->uid, 0, 0, 0, 0, dest);
}

int mbr_uuid_for_part(const MBR* mbr, size_t part_idx, char* dest)
{
    if (!mbr || !dest || part_idx >= 4)
        return -EINVAL;

    const MBRPartition* mbrpart = &mbr->partitions[part_idx];

    u16 partchecksum = *((u16*)mbrpart);

    u32 a = mbr->uid;
    u16 b = partchecksum;
    u16 c = mbr->bootstrap[0] ^ mbr->bootstrap[1] ^ mbr->bootstrap[2] ^
            mbr->bootstrap[3];
    u16 d = mbr->bootstrap[4] ^ mbr->bootstrap[5] ^ mbr->bootstrap[6] ^
            mbr->bootstrap[7];
    u64 e = part_idx + 1;

    return uuid_format(a, b, c, d, e, dest);
}
