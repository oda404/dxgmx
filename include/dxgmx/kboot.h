/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KBOOT_H
#define _DXGMX_KBOOT_H

/* This file contains variables that have been gathered at boot using
 * architecture specific ways, but may be used later on by other code. I hate
 * global variables, but I don't think there another *more* sane solution. */

#include <dxgmx/types.h>

extern ptr _kboot_framebuffer_paddr;
extern size_t _kboot_framebuffer_width;
extern size_t _kboot_framebuffer_height;
extern size_t _kboot_framebuffer_bpp;

#endif // !_DXGMX_KBOOT_H
