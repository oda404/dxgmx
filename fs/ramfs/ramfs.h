/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FS_RAMFS_RAMFS_H
#define _DXGMX_FS_RAMFS_RAMFS_H

#include <dxgmx/types.h>

typedef struct S_RamFsFileData
{
    void* data;
} RamFsFileData;

typedef struct S_RamFsMetadata
{
    RamFsFileData* files;
    size_t files_count;
} RamFsMetadata;

#endif // !_DXGMX_FS_RAMFS_RAMFS_H
