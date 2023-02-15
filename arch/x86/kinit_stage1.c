/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/kconfig.h>
#include <dxgmx/klog.h>
#include <dxgmx/kstdio.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/panic.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/x86/ata.h>
#include <dxgmx/x86/gdt.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/interrupts.h>
#include <dxgmx/x86/multiboot.h>
#include <dxgmx/x86/pci.h>

_INIT bool kinit_stage1()
{
    gdt_init();
    tss_init();
    idt_init();

    /* Initialize logging early on, so if something goes wrong we see it. */
    kstdio_init();
    klog_init((KLogLevel)DXGMX_CONFIG_LOG_LEVEL);
    kstdio_set_serial_debug(true);

    if (_multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
        panic("Not booted by a multiboot compliant bootloader.");

    /* Set CPU features as they may be needed for initializing system timers. */
    cpu_identify();

    mm_init();

    timekeep_init();

    ksyms_load();

    klogln(INFO, "     _");
    klogln(INFO, "  __| |_  ____ _ _ __ ___ __  __");
    klogln(INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /");
    klogln(INFO, "| (_| |>  < (_| | | | | | |>  <");
    klogln(
        INFO,
        " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s - %d.%d.%d",
        DXGMX_CODENAME,
        DXGMX_VER_MAJ,
        DXGMX_VER_MIN,
        DXGMX_PATCH_N);
    klogln(INFO, "           |___/");

    {
        struct tm date = timkeep_date();
        klogln(
            INFO,
            "Current date: %02d:%02d:%02d %02d/%02d/%d.",
            date.tm_hour,
            date.tm_min,
            date.tm_sec,
            date.tm_mday,
            date.tm_mon + 1,
            1900 + date.tm_year);
    }

    pci_enumerate_devices();

    ata_init();

    return true;
}
