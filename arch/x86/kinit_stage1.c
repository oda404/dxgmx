/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/acpi.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/mem/pageframe.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/kpaging.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/klog.h>
#include<dxgmx/kinfo.h>

int kinit_stage1()
{
    /* klog is not initiated yet and I would not recommend using it until it is. */
    /* disable interrupts until a proper idt is set up. */
    interrupts_disable();

    sysgdt_init();
    idt_init();
    rtc_init();

    interrupts_enable();

    tty_init();
    const KLogConfig config = {
        .loglevel = _DXGMX_LOGLVL_
    };
    klog_init(&config);

    klog(KLOG_INFO, "     _\n");
    klog(KLOG_INFO, "  __| |_  ____ _ _ __ ___ __  __\n");
    klog(KLOG_INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /\n");
    klog(KLOG_INFO, "| (_| |>  < (_| | | | | | |>  <\n");
    klog(KLOG_INFO, " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s - %d.%d.%d\n", _DXGMX_CODENAME_, _DXGMX_VER_MAJ_, _DXGMX_VER_MIN_, _DXGMX_PATCH_N_);
    klog(KLOG_INFO, "           |___/\n");

    if(_multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
        abandon_ship("Not booted by a multiboot compliant bootloader\n");

    rtc_dump_time_and_date();
    cpu_identify();
    
    mmap_init();

    MultibootMBI *mbi = (MultibootMBI *)_multiboot_info_struct_base;

    for(
        MultibootMMAP *mmap = (MultibootMMAP *)mbi->mmap_base;
        (ptr)mmap < mbi->mmap_base + mbi->mmap_length;
        mmap = (MultibootMMAP *)((ptr)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_add_entry(mmap->base, mmap->length, mmap->type);
    }

    klog(KLOG_INFO, "Memory map provided by BIOS:\n");
    mmap_dump();

    /* mark the kernel itself as kreserved */
    mmap_update_entry_type(_kbase, _kend - _kbase, MMAP_RESERVED);
    /* 
     * i lose a bit of available physical memory by aligning 
     * the available areas but gain a lot of mental health
     */
    mmap_align_entries(PAGE_SIZE);

    acpi_init();

    pageframe_alloc_init();
    kpaging_init();

    return 0;
}
