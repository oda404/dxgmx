
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
#include<stdio.h>

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

void kmain(unsigned long magic, unsigned long  mboot_info_addr __ATTR_MAYBE_UNUSED)
{
    tty_init();

    if(magic != BL_MAGIC)
    {
        abandon_ship("Expected boot spec " BOOT_SPEC_NAME " was not matched\n");
    }

    printf("Boot spec is %s\n", BOOT_SPEC_NAME);

    printf(
        "Codename %s version %d.%d.%d\n", 
        __KCODENAME__,
        __KVER_MAJ__,
        __KVER_MIN__,
        __KPATCH_N__
    );
}
