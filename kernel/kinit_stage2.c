/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/module.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/timekeep.h>

extern int kinit_arch();

static void kinit_print_banner()
{
    klogln(INFO, "     _");
    klogln(INFO, "  __| |_  ____ _ _ __ ___ __  __");
    klogln(INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /");
    klogln(INFO, "| (_| ||  | (_| | | | | | ||  |");
    klogln(
        INFO,
        " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s - %d.%d.%d",
        CONFIG_CODENAME,
        CONFIG_VER_MAJ,
        CONFIG_VER_MIN,
        CONFIG_PATCH_N);
    klogln(INFO, "           |___/");
}

_ATTR_NORETURN void kinit_stage2()
{
    /* Initialize early architecture stuff. */
    kinit_arch();

    /* Load the first stage of builtin modules. */
    modules_init_stage1();

    klog_init();

    /* Identify CPU so other sub systems know what they are working with. */
    cpu_identify();

    /* Bring the memory manager online, meaning kmalloc, falloc, page
     * allocation, page faults, dma, etc.. */
    mm_init();

    procm_spawn_kernel_proc();

    /* Load kernel sysmbols, now that we have kmalloc */
    ksyms_load();

    /* Second stage of modules. */
    modules_init_stage2();

    /* Bring timers online. */
    timekeep_init();

    kinit_print_banner();

    modules_init_stage3();

    modules_dump_builtins();

    syscalls_init();

    vfs_init();

    procm_init();

    procm_spawn_init();

    /* Let it rip */
    procm_sched_start();
}
