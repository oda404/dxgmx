/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/bootinfo.h>
#include<dxgmx/types.h>

extern u32 _kernel_base;
extern u32 _kernel_end;
extern u32 _kstack_top;
extern u32 _kstack_bot;

/* 
 * This function initiates core hardware and 
 * brings the machine in an operational state.
 * It s implementation is architecture specific 
 * and can be found in arch/<arch>/init_stage1.c .
 */
int kinit_stage1(const BootInfo *bootinfo);
/* This function expects the hardware to be initialized
 * and in working order. It is responsible for initiating
 * kernel specific stuff.
 */
int kinit_stage2();

void kmain(u32 blmagic, u32 blinfo_base)
{
    {
        BootInfo bootinfo;
        bootinfo.kernel_base = (u32)&_kernel_base;
        bootinfo.kernel_end  = (u32)&_kernel_end;
        bootinfo.kstack_top  = (u32)&_kstack_top;
        bootinfo.kstack_bot  = (u32)&_kstack_bot;
        bootinfo.blmagic     = blmagic;
        bootinfo.blinfo_base = blinfo_base;

        kinit_stage1(&bootinfo);
    }

    kinit_stage2();
    
    for(;;)
        asm volatile("hlt");
}
