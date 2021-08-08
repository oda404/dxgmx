
#include<dxgmx/x86/sysidt.h>
#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/rtc.h>
#include<dxgmx/x86/acpi.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/bootinfo.h>
#include<dxgmx/mem/map.h>
#include<dxgmx/cpu.h>
#include<dxgmx/kdefs.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/mem/pageframe.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/kpaging.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/klog.h>
#include<stdint.h>

int kinit_stage1(const BootInfo *bootinfo)
{
    /* klog is not initiated yet and I would not recommend using it until it is. */
    tty_init();
    sysgdt_init();
    sysidt_init();
    rtc_init();

    const KLogConfig config = {
        .loglevel = KLOG_FATAL
    };
    klog_init(&config);

    klog_notag(KLOG_INFO, "     _                          \n");
    klog_notag(KLOG_INFO, "  __| |_  ____ _ _ __ ___ __  __\n");
    klog_notag(KLOG_INFO, " / _` \\ \\/ / _` | '_ ` _ \\\\ \\/ /\n");
    klog_notag(KLOG_INFO, "| (_| |>  < (_| | | | | | |>  <\n");
    klog_notag(KLOG_INFO, " \\__,_/_/\\_\\__, |_| |_| |_/_/\\_\\ %s ~ %d.%d.%d\n", _DXGMX_CODENAME_, _DXGMX_VER_MAJ_, _DXGMX_VER_MIN_, _DXGMX_PATCH_N_);
    klog_notag(KLOG_INFO, "           |___/                \n\n");

    if(bootinfo->blmagic != MULTIBOOT_BOOTLOADER_MAGIC)
        abandon_ship("Not booted by a multiboot compliant bootloader\n");

    rtc_dump_time_and_date();
    acpi_init();

    klog(
        KLOG_INFO,
        "Kernel physical base: 0x%08lX, size: 0x%08lX\n",
        bootinfo->kernel_base,
        bootinfo->kernel_end - bootinfo->kernel_base
    );
    klog(
        KLOG_INFO,
        "Kernel physical stack top: 0x%08lX, bottom: 0x%08lX\n",
        bootinfo->kstack_top,
        bootinfo->kstack_bot
    );

    cpu_identify();
    mmap_init();

    MultibootMBI *mbi = (MultibootMBI *)bootinfo->blinfo_base;
    MultibootMMAP *mmap;

    for(
        mmap = (MultibootMMAP *)mbi->mmap_base;
        (uint32_t)mmap < mbi->mmap_base + mbi->mmap_length;
        mmap = (MultibootMMAP *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_entry_add(mmap->base, mmap->length, mmap->type);
    }

    /* mark the kernel itself as kreserved */
    mmap_area_mark_kreserved(bootinfo->kernel_base, bootinfo->kernel_end - bootinfo->kernel_base);
    /* 
     * i lose a bit of available physical memory by aligning 
     * the available areas but gain a lot of mental health
     */
    mmap_entries_align(PAGE_SIZE);
    mmap_dump();

    pageframe_alloc_init();
    kpaging_init();

    return 0;
}
