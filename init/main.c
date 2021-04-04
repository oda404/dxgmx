

#include<dxgmx/purgatory.h>

#if defined(__X86__)
#include<dxgmx/x86/mboot.h>
#endif // defined(__X86__)

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

#include<dxgmx/gcc/attrs.h>
#include<dxgmx/video/tty.h>

void kmain(unsigned long magic, unsigned long  mboot_info_addr __ATTR_MAYBE_UNUSED)
{
    tty_init();
    
    switch(magic)
    {
    case MBOOT2_BOOTLOADER_MAGIC:
        tty_print("Booted by a multiboot2 bootloader.");
        break;

    case MBOOT_BOOTLOADER_MAGIC:
        tty_print("Booted by a multiboot bootloader.");
        break;
    
    default:
        tty_print("Panic: Not booted by a multiboot compliant booloader.");
        purgatory_enter();
        break;
    }

    tty_print("Codename " __KCODENAME__);
}
