
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

#include<dxgmx/purgatory.h>
#include<dxgmx/gcc/attrs.h>
#include<dxgmx/video/tty.h>
#include<stdio.h>

#if defined(__X86__)
#include<dxgmx/x86/mboot.h>
#endif // defined(__X86__)

void kmain(unsigned long magic, unsigned long  mboot_info_addr __ATTR_MAYBE_UNUSED)
{
    tty_init();
    
    switch(magic)
    {
    case MBOOT2_BOOTLOADER_MAGIC:
        printf("Booted by a multiboot2 bootloader\n");
        break;

    case MBOOT_BOOTLOADER_MAGIC:
        printf("Booted by a multiboot bootloader\n");
        break;
    
    default:
        printf("Panic: Not booted by a multiboot compliant booloader\n");
        purgatory_enter();
        break;
    }

    printf(
        "Codename %s version %d.%d.%d\n", 
        __KCODENAME__,
        __KVER_MAJ__,
        __KVER_MIN__,
        __KPATCH_N__
    );
}
