/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/acpi.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/mem/pgframe_alloc.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/klog.h>
#include<dxgmx/kinfo.h>

int kinit_stage1()
{
    /* Hang interrupt until a predictable gdt/idt is set up. */
    interrupts_disable();

    gdt_init();
    idt_init();

    /* Back in business. */
    interrupts_enable();
    
    rtc_init();

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

    /* Mark the first page as reserved. */
    mmap_update_entry_type(0, PAGE_SIZE, MMAP_RESERVED);
    /* Mark the kernel image as reserved. */
    mmap_update_entry_type(
        kinfo_get_kbase(), 
        kinfo_get_kend() - kinfo_get_kbase(), 
        MMAP_RESERVED
    );
    /* Align all entries' bases on PAGE_SIZE bytes */
    mmap_align_entries(PAGE_SIZE);

    /* Init ACPI while paging is not yet enabled, because acpi needs to look around different parts of memory. */
    acpi_init();

    pgframe_alloc_init();
    paging_init();

    rtc_dump_time_and_date();
    cpu_identify();

    return 0;
}
