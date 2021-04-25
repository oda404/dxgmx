/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/abandon_ship.h>
#include<dxgmx/stdio.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/video/tty.h>
#include<stdint.h>

#if defined(__X86__)
#include<dxgmx/x86/mboot.h>
#include<dxgmx/x86/mboot2.h>
#endif // __X86__

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

void kmain(uint32_t bl_magic, uint32_t bl_info_base)
{
    tty_init();

    BootInfo bootinfo;

    switch(bl_magic)
    {
    case MBOOT2_BOOTLOADER_MAGIC:
        kprintf("Boot spec is multiboot2\n");
        break;

    case MBOOT_BOOTLOADER_MAGIC:
        kprintf("Boot spec is multiboot\n");
        break;

    default:
        abandon_ship("Not booted by a supported bootloader\n");
        break;
    }

    kinit_stage1(&bootinfo);
    kinit_stage2();
    
    for(;;)
        asm volatile("hlt");
}
