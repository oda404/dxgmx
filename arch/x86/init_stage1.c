
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/stdio.h>
#include<stdint.h>

extern uint32_t kernel_addr_base;
extern uint32_t kernel_addr_end;

int kinit_stage1(const BootInfo *bootinfo)
{
    kprintf("Going for target: kinit_stage1.\n");
    kprintf(
        "Codename %s version %d.%d.%d\n", 
        __KCODENAME__,
        __KVER_MAJ__,
        __KVER_MIN__,
        __KPATCH_N__
    );

    kprintf(
        "Kernel base: 0x%X, size: 0x%X\n",
        &kernel_addr_base,
        &kernel_addr_end - &kernel_addr_base
    );

    gdt_init();
    idt_init();

    kprintf("Reached target: kinit_stage1.\n");

    return 0;
}
