/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/kconfig.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/panic.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/video/fb.h>
#include <dxgmx/video/psf.h>
#include <dxgmx/x86/ata.h>
#include <dxgmx/x86/gdt.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/interrupts.h>
#include <dxgmx/x86/multiboot.h>
#include <dxgmx/x86/pci.h>

#define KLOGF_PREFIX "kinit_stage1: "

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

    MultibootMBI* mbi =
        (MultibootMBI*)(_multiboot_info_struct_base + kimg_map_offset());

    if (mbi->fb.type == 1)
    {
        ptr paddr = mbi->fb.base;
        size_t width = mbi->fb.width;
        size_t height = mbi->fb.height;
        size_t bpp = mbi->fb.bpp;

        int st = fb_init(paddr, width, height, bpp);
        if (st < 0)
        {
            KLOGF(
                ERR,
                "Failed to initialize framebuffer at 0x%p (%dx%d:%d), st: %d.",
                (void*)paddr,
                width,
                height,
                bpp,
                st);
        }
        else
        {
            kstdio_init_fb();
        }
    }

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
