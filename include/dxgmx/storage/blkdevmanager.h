/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STORAGE_BLKDEVMANAGER_H
#define _DXGMX_STORAGE_BLKDEVMANAGER_H

#include <dxgmx/storage/blkdev.h>
#include <dxgmx/types.h>

/* The blkdevmanager prefix sounds kinda weird */

bool blkdevmanager_init();
int blkdevmanager_register_dev(BlockDevice* dev);
int blkdevmanager_unregister_dev(BlockDevice* dev);

/* In this case id could be a path like /dev/hdap0 or an uuid like UUID=.... */
const BlockDevice* blkdevm_find_blkdev_by_id(const char* id);

const BlockDevice* blkdevmanager_find_blkdev_by_name(const char* name);
const BlockDevice* blkdevmanager_find_blkdev_by_uuid(const char* uuid);

#endif // !_DXGMX_STORAGE_BLKDEVMANAGER_H
