/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/kboot.h>
#include <dxgmx/klog.h>
#include <dxgmx/ksyms.h>
#include <dxgmx/module.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/timekeep.h>

static void kinit_print_banner()
{
    klogln(INFO, "\x1b[0;35;49m     _");
    klogln(
        INFO,
        "\x1b[0;35;49m  __| |\x1b[0;36;49m_  __\x1b[0;35;49m__ _ _ __ ___ __  __");

    klogln(
        INFO,
        "\x1b[0;35;49m / _` \x1b[0;36;49m\\ \\/ /\x1b[0;35;49m _` | '_ ` _ \\\\ \\/ /");

    klogln(
        INFO,
        "\x1b[0;35;49m| (_| |\x1b[0;36;49m|  |\x1b[0;35;49m (_| | | | | | ||  |");

    klogln(
        INFO,
        "\x1b[0;35;49m \\__,_\x1b[0;36;49m/_/\\_\\\x1b[0;35;49m__, |_| |_| |_/_/\\_\\ \x1b[0;36;49m%s - %d.%d.%d",
        CONFIG_CODENAME,
        CONFIG_VER_MAJ,
        CONFIG_VER_MIN,
        CONFIG_PATCH_N);

    klogln(INFO, "\x1b[0;35;49m           |___/");
}

_ATTR_NORETURN void kinit_stage2()
{
    /* Initialize early architecture stuff. */
    extern void kinit_arch();
    kinit_arch();

    /* Load the first stage of builtin modules. */
    mod_builtin_init_stage1();

    klog_init();

    /* Let's hope nothing needs kboot variables before this */
    kbootinfo_parse();

    /* Identify CPU so other sub systems know what they are working with. */
    cpu_identify();

    /* Bring the memory manager online, meaning kmalloc, falloc, page
     * allocation, page faults, dma, etc.. */
    mm_init();

    /* Second stage of modules. */
    mod_builtin_init_stage2();

    /* Load kernel sysmbols, now that we have kmalloc */
    ksyms_load();

    /* Bring timers online. */
    timekeep_init();

    kinit_print_banner();

    mod_builtin_init_stage3();

    mod_builtin_dump_all();

    syscalls_init();

    vfs_init();

    procm_init();

    procm_spawn_init();

    /* Let it rip */
    procm_sched_start();
}
