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
#include<dxgmx/panic.h>
#include<dxgmx/klog.h>
#include<dxgmx/timer.h>
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/x86/acpi.h>

int kinit_stage1()
{
    /* Hang interrupts until a predictable gdt/idt is set up. */
    interrupts_disable();

    gdt_init();
    idt_init();

    /* Back in business. */
    interrupts_enable();

    tty_init();
    klog_init((KLogLevel)_DXGMX_LOGLVL_);

    acpi_reserve_tables();
    cpu_identify();
    
    pit_init();
    pit_enable_periodic_int();
    rtc_init();
    rtc_enable_periodic_int();
    timer_find_src();

    klog_try_exit_early();

    klogln(INFO, "     _");
    klogln(INFO, "  __| |_  ____ _ _ __ ___ __  __");
    klogln(INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /");
    klogln(INFO, "| (_| |>  < (_| | | | | | |>  <");
    klogln(INFO, " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s - %d.%d.%d", _DXGMX_CODENAME_, _DXGMX_VER_MAJ_, _DXGMX_VER_MIN_, _DXGMX_PATCH_N_);
    klogln(INFO, "           |___/");

    if(_multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Not booted by a multiboot compliant bootloader.");

    mmanager_init();

    rtc_dump_date();

    return 0;
}
