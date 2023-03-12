/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kboot.h>
#include <dxgmx/kimg.h>
#include <dxgmx/x86/gdt.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/multiboot.h>

int kinit_arch()
{
    gdt_init();
    tss_init();
    idt_init();

    MultibootMBI* mbi =
        (MultibootMBI*)(_multiboot_info_struct_base + kimg_map_offset());

    if (mbi->fb.type == 1)
    {
        _kboot_framebuffer_paddr = mbi->fb.base;
        _kboot_framebuffer_width = mbi->fb.width;
        _kboot_framebuffer_height = mbi->fb.height;
        _kboot_framebuffer_bpp = mbi->fb.bpp;
    }

    if ((mbi->flags & (1 << 3)) && mbi->mods_count > 0)
    {
        const MultibootModule* hdr =
            (const MultibootModule*)(mbi->mods_base + kimg_map_offset());
        _kboot_initrd_paddr = hdr->start;
        _kboot_initrd_size = hdr->end - hdr->start;
        _kboot_has_initrd = true;
    }

    return 0;
}
