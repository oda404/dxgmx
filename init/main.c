/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/abandon_ship.h>
#include<dxgmx/stdio.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/video/tty.h>
#include<stdint.h>

extern uint32_t _kernel_base;
extern uint32_t _kernel_end;
extern uint32_t _kstack_top;
extern uint32_t _kstack_bot;

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

void kmain(uint32_t blmagic, uint32_t blinfo_base)
{
    tty_init();

    {
        BootInfo bootinfo;
        bootinfo.kernel_base = (uint32_t)&_kernel_base;
        bootinfo.kernel_end  = (uint32_t)&_kernel_end;
        bootinfo.kstack_top  = (uint32_t)&_kstack_top;
        bootinfo.kstack_bot  = (uint32_t)&_kstack_bot;
        bootinfo.blmagic     = blmagic;
        bootinfo.blinfo_base = blinfo_base;

        kinit_stage1(&bootinfo);
    }

    kinit_stage2();
    
    for(;;)
        asm volatile("hlt");
}
