
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/mboot.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/mmap.h>
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

    /* start putting mmap entries at phys address 0 and hope for the best */
    mmap_init(0);

    mboot_mbi *mbi = (mboot_mbi *)bootinfo->blinfo_base;
    mboot_mmap *mmap;

    for(
        mmap = (mboot_mmap *)mbi->mmap_base_addr;
        (uint32_t)mmap < mbi->mmap_base_addr + mbi->mmap_length;
        mmap = (mboot_mmap *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_add_area(mmap->base_addr, mmap->length, mmap->type);
    }

    const MemoryMap *mmapa = mmap_get_full_map();

    for(
        size_t i = 0;
        i < mmapa->areas_cnt;
        ++i
    )
    {
        kprintf(
            "base: 0x%X | size: 0x%X | type: %d\n",
            (uint32_t)mmapa->areas[i].base,
            (uint32_t)mmapa->areas[i].size,
            mmapa->areas[i].type
        );
    }

    gdt_init();
    idt_init();

    kprintf("Reached target: kinit_stage1.\n");

    return 0;
}
