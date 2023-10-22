/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/generated/kconfig.h>
#include <dxgmx/kboot.h>
#include <dxgmx/panic.h>

#if !defined(CONFIG_MULTIBOOT_BOOTLOADER)
#error No boot spec has been configured! How should we boot?
#endif

#ifdef CONFIG_MULTIBOOT_BOOTLOADER
#include <dxgmx/multiboot.h>
#endif

#define SYS_MEMORY_REGIONS_MAX 16
static MemoryRegion g_sys_mregs[SYS_MEMORY_REGIONS_MAX];
static MemoryRegionMap g_sys_mregmap = {
    .regions = g_sys_mregs,
    .regions_capacity = SYS_MEMORY_REGIONS_MAX,
};

const KernelBootInfo ___kboot_info = {.mregmap = &g_sys_mregmap};

void kbootinfo_parse()
{
    /* Stinky and unpleasant const discard cast. We can do shit like
     * this here because the mm has not yet enforced section permissions, and
     * ___kbot_info should not (nor can it) be modified after this function
     * exits (and mm enforces permissions). */
    KernelBootInfo* kboot_info = (KernelBootInfo*)&___kboot_info;

#ifdef CONFIG_MULTIBOOT_BOOTLOADER
    if (multiboot_parse_info(kboot_info) == 0)
        return;
#endif

    panic("Could not figure out the bootloader spec!");
}
