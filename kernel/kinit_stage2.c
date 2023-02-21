/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/kconfig.h>
#include <dxgmx/klog.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/module.h>
#include <dxgmx/panic.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/linkedlist.h>
#include <dxgmx/video/fb.h>

extern int kinit_arch();

static void kinit_print_banner()
{
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
}

_ATTR_NORETURN void kinit_stage2()
{
    /* Initialize early architecture stuff. */
    kinit_arch();

    /* Load the first stage of builtin modules. */
    modules_init_stage1();

    klog_init(DXGMX_CONFIG_LOG_LEVEL);

    /* Identify CPU so other sub systems know what they are working with. */
    cpu_identify();

    /* Bring the memory manager online, meaning kmalloc, falloc, page
     * allocation, page faults, dma, etc.. */
    mm_init();

    /* Try to initialize the framebuffer. I think this should be a module. */
    fb_init();

    /* Load kernel sysmbols, now that we have kmalloc */
    ksyms_load();

    /* Second stage of modules. */
    modules_init_stage2();

    /* Bring timers online. */
    timekeep_init();

    kinit_print_banner();

    modules_init_stage3();

    modules_dump_builtins();

    if (syscalls_init() < 0)
        panic("Failed to setup system calls. Not proceeding!");

    if (vfs_init() < 0)
        panic("Failed to initialize VFS!");

    /* Spawn PID 1. Here, we only load the binary in memory and have it ready to
     * run, waiting for the scheduler to kick in. For details on how this binary
     * works check out kernel/kinit_stage3.c */
    if (procm_spawn_init() != 1)
        panic("Failed to spawn PID 1!");

    /* Let it rip */
    sched_init();
}
