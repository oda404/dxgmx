/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FB_USER_H
#define _DXGMX_FB_USER_H

#ifdef _KERNEL
#include <posix/sys/types.h>
#else
#include <sys/types.h>
#endif

#define FBIO_GET_INFO 0xFF01
typedef struct FBInfo
{
    size_t width;
    size_t height;
    uint8_t bpp;
} FBInfo;

#endif // !_DXGMX_FB_USER_H
