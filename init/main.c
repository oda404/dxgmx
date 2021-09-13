/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/bootinfo.h>
#include<dxgmx/types.h>
#include<dxgmx/attrs.h>
#include<dxgmx/kstack.h>

extern u32 _kernel_base;
extern u32 _kernel_end;

/* 
 * This function initiates core hardware and 
 * brings the machine in an operational state.
 * It's implementation is architecture specific 
 * and can be found in arch/<arch>/kinit_stage1.c.
 */
extern int kinit_stage1(const BootInfo *bootinfo);
/* This function expects the hardware to be initialized
 * and in working order. It is responsible for initiating
 * kernel specific stuff. It's implementation can be found in 
 * kernel/kinit_stage2.c
 */
extern int kinit_stage2();

_ATTR_NORETURN void kmain(u32 blmagic, u32 blinfo_base)
{
    {
        BootInfo bootinfo;
        bootinfo.kernel_base = (u32)&_kernel_base;
        bootinfo.kernel_end  = (u32)&_kernel_end;
        bootinfo.kstack_top  = _kstack_top;
        bootinfo.kstack_bot  = _kstack_bot;
        bootinfo.blmagic     = blmagic;
        bootinfo.blinfo_base = blinfo_base;

        kinit_stage1(&bootinfo);
    }

    /**
     * FIXME: kinit_stage2 is not marked with _ATTR_NORETURN,
     * even though it (or some other kinit_stage3 function ?) should.
    */
    kinit_stage2();
    /**
     * kmain should never return. Worst case scenario abandon_ship
     * gets called in kinit_stage2.
    */
    __builtin_unreachable();
}
