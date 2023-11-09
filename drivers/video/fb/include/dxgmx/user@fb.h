/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_FB_USER_H
#define _DXGMX_FB_USER_H

#include <dxgmx/user@types.h>

#define FBIO_GET_INFO 0xFF01
typedef struct FBInfo
{
    size_t width;
    size_t height;
    _u8 bpp;
} FBInfo;

#endif // !_DXGMX_FB_USER_H
