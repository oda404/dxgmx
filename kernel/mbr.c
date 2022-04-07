/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/storage/mbr.h>
#include <dxgmx/string.h>

int mbr_read(BlockDevice* dev, MBR* mbr_out)
{
    if (!dev || !mbr_out)
        return -EINVAL;

    if (!dev->read(dev, 0, 1, mbr_out))
        return -EIO;

    return 0;
}
