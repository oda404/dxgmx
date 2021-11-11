/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/panic.h>
#include<dxgmx/klog.h>
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/timekeep.h>

int kinit_stage1()
{
    /* Hang interrupts until a predictable gdt/idt is set up. */
    interrupts_disable();

    gdt_init();
    idt_init();

    /* Back in business. */
    interrupts_enable();

    /* Initialize logging early on, so if something goes wrong we see it. */
    tty_init();
    klog_init((KLogLevel)_DXGMX_LOGLVL_);

    if(_multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Not booted by a multiboot compliant bootloader.");

    /* Set up memory management. */
    mmanager_init();

    /* Set CPU features as they may be needed for initializing system timers. */
    cpu_identify();

    /* Start timekeeping and get out of 'early' mode. */
    timekeep_init();

    klogln(INFO, "     _");
    klogln(INFO, "  __| |_  ____ _ _ __ ___ __  __");
    klogln(INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /");
    klogln(INFO, "| (_| |>  < (_| | | | | | |>  <");
    klogln(INFO, " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s - %d.%d.%d", _DXGMX_CODENAME_, _DXGMX_VER_MAJ_, _DXGMX_VER_MIN_, _DXGMX_PATCH_N_);
    klogln(INFO, "           |___/");

    {
        struct tm date = timkeep_date();
        klogln(INFO, "Current date: %02d:%02d:%02d %02d/%02d/%d.", date.tm_hour, date.tm_min, date.tm_sec, date.tm_mday, date.tm_mon + 1, 1900 + date.tm_year);
    }

    return 0;
}
