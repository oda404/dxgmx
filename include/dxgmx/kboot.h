/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_KBOOT_H
#define _DXGMX_KBOOT_H

/* This file contains variables that have been gathered at boot by the
 * bootloader, but may/will be used later on by other code and would be a pain
 * to get at that specific point. I hate global variables, but I don't think
 * there is another *more* sane solution. */

#include <dxgmx/mem/mregmap.h>
#include <dxgmx/types.h>

typedef struct KernelBootInfo
{
    bool has_fb;
    ptr fb_pa;
    ptr fb_width;
    ptr fb_height;
    ptr fb_bpp;

    MemoryRegionMap* mregmap;
} KernelBootInfo;

extern const KernelBootInfo ___kboot_info;

void kbootinfo_parse();

#endif // !_DXGMX_KBOOT_H
