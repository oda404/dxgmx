
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/mboot.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/mmap.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/paging/frame.h>
#include<dxgmx/paging/size.h>
#include<dxgmx/kprintf.h>
#include<stdint.h>

int kinit_stage1(const BootInfo *bootinfo)
{
    kprintf("Targeting: kinit_stage1.\n");

    if(bootinfo->blmagic != MBOOT_BOOTLOADER_MAGIC)
        abandon_ship("Not booted by a multiboot compliant bootloader\n");

    kprintf(
        "dxgmx - %s %d.%d.%d\n", 
        _DXGMX_CODENAME_,
        _DXGMX_VER_MAJ_,
        _DXGMX_VER_MIN_,
        _DXGMX_PATCH_N_
    );

    kprintf(
        "Kernel base: 0x%lX, size: 0x%lX\n",
        bootinfo->kernel_base,
        bootinfo->kernel_end - bootinfo->kernel_base
    );
    kprintf(
        "Kernel stack top: 0x%lX, bottom: 0x%lX\n",
        bootinfo->kstack_top,
        bootinfo->kstack_bot
    );

    mmap_init();

    mboot_mbi *mbi = (mboot_mbi *)bootinfo->blinfo_base;
    mboot_mmap *mmap;

    for(
        mmap = (mboot_mmap *)mbi->mmap_base_addr;
        (uint32_t)mmap < mbi->mmap_base_addr + mbi->mmap_length;
        mmap = (mboot_mmap *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_entry_add(mmap->base_addr, mmap->length, mmap->type);
    }

    /* mark the kernel itself as kreserved */
    mmap_area_mark_kreserved(bootinfo->kernel_base, bootinfo->kernel_end - bootinfo->kernel_base);
    /* 
     * i lose a bit of available physical memory by aligning 
     * the available areas but gain a lot of mental health
     */
    mmap_entries_align(_PAGE_SIZE);
    mmap_print();

    pageframe_alloc_init();

    gdt_init();
    idt_init();

    kprintf("Reached target: kinit_stage1.\n");

    return 0;
}
