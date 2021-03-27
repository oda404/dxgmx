

#include<dxgmx/tty/tty.h>
#include<dxgmx/kio/kio.h>
#include<dxgmx/video/vga_text.h>
#include<dxgmx/purgatory.h>

#if defined(__X86__)
#include<dxgmx/x86/mboot.h>
#endif // defined(__X86__)

#ifndef __K_VER_MAJ__
#define __K_VER_MAJ__ -1
#endif
#ifndef __K_VER_MIN__
#define __K_VER_MIN__ -1
#endif
#ifndef __K_PATCH_N__
#define __K_PATCH_N__ -1
#endif
#ifndef __K_CODENAME__
#define __K_CODENAME__ "undefined"
#endif

void kmain(unsigned long magic, unsigned long mboot_info_addr)
{
    if(magic != MBOOT_BOOTLOADER_MAGIC)
    {
        /* has not been booted by a multi boot compliant bootloader, and we don t support that */
        vga_put_str("Panic: Not booted by a multi boot compliant booloader.", VGA_COLOR_WHITE, VGA_COLOR_BLACK, 0);
        purgatory_enter();
    }

    vga_put_str("Booting codename: " __K_CODENAME__, VGA_COLOR_WHITE, VGA_COLOR_BLACK, 1);
}
