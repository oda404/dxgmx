
#include<dxgmx/x86/sysidt.h>
#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/mem/map.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/mem/pageframe.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/kpaging.h>
#include<dxgmx/kprintf.h>
#include<stdint.h>

int kinit_stage1(const BootInfo *bootinfo)
{
    kprintf("     _                          \n");
    kprintf("  __| |_  ____ _ _ __ ___ __  __\n");
    kprintf(" / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /\n");
    kprintf("| (_| |>  < (_| | | | | | |>  <\n");
    kprintf(" \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s ~ %d.%d.%d\n",_DXGMX_CODENAME_, _DXGMX_VER_MAJ_, _DXGMX_VER_MIN_, _DXGMX_PATCH_N_);
    kprintf("           |___/                \n");
    kprintf("\n");

    if(bootinfo->blmagic != MULTIBOOT_BOOTLOADER_MAGIC)
        abandon_ship("Not booted by a multiboot compliant bootloader\n");

    kprintf(
        "Kernel physical base: 0x%lX, size: 0x%lX\n",
        bootinfo->kernel_base,
        bootinfo->kernel_end - bootinfo->kernel_base
    );
    kprintf(
        "Kernel physical stack top: 0x%lX, bottom: 0x%lX\n",
        bootinfo->kstack_top,
        bootinfo->kstack_bot
    );

    mmap_init();

    MultibootMBI *mbi = (MultibootMBI *)bootinfo->blinfo_base;
    MultibootMMAP *mmap;

    for(
        mmap = (MultibootMMAP *)mbi->mmap_base;
        (uint32_t)mmap < mbi->mmap_base + mbi->mmap_length;
        mmap = (MultibootMMAP *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_entry_add(mmap->base, mmap->length, mmap->type);
    }

    /* mark the kernel itself as kreserved */
    mmap_area_mark_kreserved(bootinfo->kernel_base, bootinfo->kernel_end - bootinfo->kernel_base);
    /* 
     * i lose a bit of available physical memory by aligning 
     * the available areas but gain a lot of mental health
     */
    mmap_entries_align(PAGE_SIZE);
    mmap_dump();

    pageframe_alloc_init();

    sysgdt_init();
    sysidt_init();
    kpaging_init();

    return 0;
}
