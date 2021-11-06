/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/x86/pit.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<dxgmx/kinfo.h>
#include<dxgmx/timer.h>
#include<dxgmx/mem/mmanager.h>

int kinit_stage1()
{
    /* Hang interrupts until a predictable gdt/idt is set up. */
    interrupts_disable();

    gdt_init();
    idt_init();

    /* Back in business. */
    interrupts_enable();
    
    pit_init();
    pit_enable_periodic_int();
    rtc_init();
    rtc_enable_periodic_int();
    timer_find_src();

    tty_init();
    const KLogConfig config = {
        .loglevel = _DXGMX_LOGLVL_
    };
    klog_init(&config);

    klog(INFO, "     _\n");
    klog(INFO, "  __| |_  ____ _ _ __ ___ __  __\n");
    klog(INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /\n");
    klog(INFO, "| (_| |>  < (_| | | | | | |>  <\n");
    klog(INFO, " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s - %d.%d.%d\n", _DXGMX_CODENAME_, _DXGMX_VER_MAJ_, _DXGMX_VER_MIN_, _DXGMX_PATCH_N_);
    klog(INFO, "           |___/\n");

    if(_multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
        abandon_ship("Not booted by a multiboot compliant bootloader\n");

    mmanager_init();

    rtc_dump_date();
    cpu_identify();

    return 0;
}
