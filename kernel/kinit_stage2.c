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
#include <dxgmx/proc/procm.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/timekeep.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/linkedlist.h>

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
        DXGMX_CODENAME,
        DXGMX_VER_MAJ,
        DXGMX_VER_MIN,
        DXGMX_PATCH_N);
    klogln(INFO, "           |___/");
}

_ATTR_NORETURN void kinit_stage2()
{
    /* Initialize early architecture stuff. */
    kinit_arch();

    /* Bring early timers in a known, but "dormant" state. */
    timekeep_early_init();

    /* Load the first stage of builtin modules. */
    modules_init_stage1();

    klog_init(DXGMX_CONFIG_LOG_LEVEL);

    /* Identify CPU so other sub systems know what they are working with. */
    cpu_identify();

    /* Bring the memory manager online, meaning kmalloc, falloc, page
     * allocation, page faults, dma, etc.. */
    mm_init();

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

    procm_spawn_kernel_proc();
    procm_spawn_init();

    /* Let it rip */
    sched_init();
}
