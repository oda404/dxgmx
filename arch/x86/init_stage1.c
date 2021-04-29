
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/mboot.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/stdio.h>
#include<stdint.h>

int kinit_stage1(const BootInfo *bootinfo)
{
    kprintf("Targeting: kinit_stage1.\n");

    if(bootinfo->blmagic != MBOOT_BOOTLOADER_MAGIC)
        abandon_ship("Not booted by a multiboot compliant bootloader\n");

    kprintf(
        "Codename %s version %d.%d.%d\n", 
        __KCODENAME__,
        __KVER_MAJ__,
        __KVER_MIN__,
        __KPATCH_N__
    );

    kprintf(
        "Kernel base: 0x%X, size: 0x%X\n",
        bootinfo->kernel_base,
        bootinfo->kernel_end - bootinfo->kernel_base
    );
    kprintf(
        "Kernel stack top: 0x%X, bottom: 0x%X\n",
        bootinfo->kstack_top,
        bootinfo->kstack_bot
    );

    gdt_init();
    idt_init();

    kprintf("Reached target: kinit_stage1.\n");

    return 0;
}
