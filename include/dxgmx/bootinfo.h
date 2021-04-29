
#ifndef __DXGMX_BOOTINFO_H__
#define __DXGMX_BOOTINFO_H__

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

#endif // __DXGMX_BOOTINFO_H__
