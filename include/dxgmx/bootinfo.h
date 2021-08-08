/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_BOOTINFO_H
#define _DXGMX_BOOTINFO_H

#include<stdint.h>

typedef struct S_BootInfo
{
    uint32_t kernel_base;
    uint32_t kernel_end;
    uint32_t kstack_top;
    uint32_t kstack_bot;
    uint32_t blmagic;
    uint32_t blinfo_base;
} BootInfo;

#endif // _DXGMX_BOOTINFO_H
