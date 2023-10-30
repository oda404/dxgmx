/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_VIDEO_FB_H
#define _DXGMX_VIDEO_FB_H

#include <dxgmx/types.h>

typedef struct S_FrameBuffer
{
    ptr base_pa;
    ptr base_va;
    size_t width;
    size_t height;
    size_t bpp;
    size_t bytespp;

    bool takeover;
} FrameBuffer;

typedef struct FrameBufferPack
{
    FrameBuffer** fbs;
    size_t* fb_count;
} FrameBufferPack;

FrameBufferPack* fb_get_pack();
int fb_write_pixel(size_t x, size_t y, u32 pixel, FrameBuffer* fb);

#endif // !_DXGMX_VIDEO_FB_H
