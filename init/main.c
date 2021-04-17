
#ifndef __KVER_MAJ__
#define __KVER_MAJ__ -1
#endif
#ifndef __KVER_MIN__
#define __KVER_MIN__ -1
#endif
#ifndef __KPATCH_N__
#define __KPATCH_N__ -1
#endif
#ifndef __KCODENAME__
#define __KCODENAME__ "undefined"
#endif

#include<dxgmx/abandon_ship.h>
#include<dxgmx/gcc/attrs.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/kstdio.h>
#include<stdint.h>

#if defined(__MBOOT2__)

#include<dxgmx/x86/mboot2.h>
#define BL_MAGIC MBOOT2_BOOTLOADER_MAGIC
#define BOOT_SPEC_NAME "multiboot2"

#elif defined(__MBOOT__)

#include<dxgmx/x86/mboot.h>
#define BL_MAGIC MBOOT_BOOTLOADER_MAGIC
#define BOOT_SPEC_NAME "multiboot"

#else // standalone

#endif // defined(__MBOOT2__)

extern uint32_t kernel_addr_base;
extern uint32_t kernel_addr_end;

void kmain(uint32_t magic, uint32_t mbi_base_addr)
{
    tty_init();

    kprintf(
        "Codename %s version %d.%d.%d\n", 
        __KCODENAME__,
        __KVER_MAJ__,
        __KVER_MIN__,
        __KPATCH_N__
    );

    kprintf(
        "Kernel base: 0x%X, size: 0x%X\n",
        &kernel_addr_base,
        &kernel_addr_end - &kernel_addr_base
    );

    if(magic != BL_MAGIC)
    {
        abandon_ship("Expected boot spec " BOOT_SPEC_NAME " was not matched\n");
    }

    kprintf("Boot spec is %s\n", BOOT_SPEC_NAME);

    mboot_mbi *mbi = (mboot_mbi*)mbi_base_addr;
    mboot_mmap *mmap = NULL;
    kprintf("\nmemory map: \n");
    for(
        mmap = (mboot_mmap *)mbi->mmap_base_addr;
        (U32)mmap < mbi->mmap_base_addr + mbi->mmap_length;
        mmap = (mboot_mmap *)((U32)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        kprintf("base 0x%X ", mmap->base_addr);
        kprintf("len 0x%X ", mmap->length);
        kprintf("type %d ", mmap->type);
        kprintf("size %d\n", mmap->size);
    }
}
