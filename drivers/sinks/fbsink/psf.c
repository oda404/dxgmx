/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include "psf.h"
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/string.h>

/* You can link a PSF(U) file into the kernel if you want to display text on a
 * framebuffer. The following symbols need to point to the start/end of the PSF
 * file. */

/* Define these as weak, so we don't get a linktime error even if we don't link
 * with a font */
_WEAK u8 _builtin_psfu_start[32];
_WEAK u8 _builtin_psfu_end[1];

int psf_validate_builtin()
{
    PSF2Header* hdr = (PSF2Header*)_builtin_psfu_start;

    if (hdr->magic != 0x864ab572)
        return -EINVAL;

    return 0;
}

const PSF2Header* psf_get_builtin_header()
{
    return (PSF2Header*)_builtin_psfu_start;
}

int psf_get_glyph_data(char c, void* dest, const PSF2Header* hdr)
{
    u8* base = (u8*)((ptr)hdr + hdr->header_size);
    base += c * hdr->glyph_size;

    memcpy(dest, base, hdr->glyph_size);

    return 0;
}
