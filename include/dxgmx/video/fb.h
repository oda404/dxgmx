/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_VIDEO_FB_H
#define _DXGMX_VIDEO_FB_H

#include <dxgmx/types.h>

typedef struct S_FrameBuffer
{
    ptr paddr;
    ptr vaddr;
    size_t width;
    size_t height;
    size_t bpp;
    size_t bytespp;
} FrameBuffer;

int fb_init(ptr paddr, size_t width, size_t height, size_t bpp);

int fb_write_pixel(size_t x, size_t y, u32 pixel);

FrameBuffer* fb_get_main();

#endif // !_DXGMX_VIDEO_FB_H
