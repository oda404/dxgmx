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

    return 0;
}
