/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_VIDEO_PSF_H
#define _DXGMX_VIDEO_PSF_H

#include <dxgmx/types.h>

typedef struct S_PSF2Header
{
    u32 magic;
    u32 version;
    u32 header_size;
    u32 flags;
    u32 glyph_count;
    u32 glyph_size;
    u32 glyph_height;
    u32 glyph_width;
} PSF2Header;

/**
 *
 */
int psf_validate_builtin();

/**
 * Get the PSF header for the builtin font. Doesn't do error checking, you
 * should check if the builtin font is valid using psf_validate_builtin().
 */
const PSF2Header* psf_get_builtin_header();

int psf_get_glyph_data(char c, void* dest, const PSF2Header* hdr);

#endif // !_DXGMX_VIDEO_PSF_H
