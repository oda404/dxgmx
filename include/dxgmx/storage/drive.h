/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_STORAGE_DRIVE_H
#define _DXGMX_STORAGE_DRIVE_H

#include<dxgmx/types.h>

typedef bool(*storage_drive_read)(u64 start, size_t bytes, u8* buf, const void *dev);
typedef bool(*storage_drive_write)(u64 start, size_t byLtes, const u8 *buf, const void *dev);

#endif // !_DXGMX_STORAGE_DRIVE_H
