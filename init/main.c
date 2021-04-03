

#include<dxgmx/video/vga_text.h>
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

void kmain(unsigned long magic, unsigned long  mboot_info_addr __ATTR_MAYBE_UNUSED)
{
    if(magic != MBOOT_BOOTLOADER_MAGIC)
    {
        /* has not been booted by a multi boot compliant bootloader, and we don t support that */
        vga_put_str("Panic: Not booted by a multi boot compliant booloader.", VGA_COLOR_WHITE, VGA_COLOR_BLACK, 0);
        purgatory_enter();
    }

    vga_put_str("Booting codename: " __KCODENAME__, VGA_COLOR_WHITE, VGA_COLOR_BLACK, 1);
}
