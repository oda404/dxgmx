/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kimg.h>
#include <dxgmx/kstack.h>

extern u8 _kernel_base[];
extern u8 _kernel_vaddr[];
extern u8 _text_sect_start[];
extern u8 _text_sect_end[];
extern u8 _rodata_sect_start[];
extern u8 _rodata_sect_end[];
extern u8 _ro_post_init_sect_start[];
extern u8 _ro_post_init_sect_end[];
extern u8 _init_sect_start[];
extern u8 _init_sect_end[];
extern u8 _data_sect_start[];
extern u8 _data_sect_end[];
extern u8 _bss_sect_start[];
extern u8 _bss_sect_end[];
extern u8 _bootloader_sect_start[];
extern u8 _bootloader_sect_end[];
extern u8 _kernel_map_offset[];
extern u8 _kernel_size[];

ptr kimg_bootloader_start()
{
    return (ptr)_bootloader_sect_start;
}

ptr kimg_bootloader_end()
{
    return (ptr)_bootloader_sect_end;
}

ptr kimg_text_start()
{
    return (ptr)_text_sect_start;
}

ptr kimg_text_end()
{
    return (ptr)_text_sect_end;
}

ptr kimg_stack_top()
{
    return (ptr)_kernel_stack_top;
}

ptr kimg_stack_bot()
{
    return (ptr)_kernel_stack_bot;
}

ptr kimg_bss_start()
{
    return (ptr)_bss_sect_start;
}
ptr kimg_bss_end()
{
    return (ptr)_bss_sect_end;
}

ptr kimg_data_start()
{
    return (ptr)_data_sect_start;
}

ptr kimg_data_end()
{
    return (ptr)_data_sect_end;
}

ptr kimg_rodata_start()
{
    return (ptr)_rodata_sect_start;
}

ptr kimg_rodata_end()
{
    return (ptr)_rodata_sect_end;
}

ptr kimg_ro_postinit_start()
{
    return (ptr)_ro_post_init_sect_start;
}

ptr kimg_ro_postinit_end()
{
    return (ptr)_ro_post_init_sect_end;
}

ptr kimg_init_start()
{
    return (ptr)_init_sect_start;
}

ptr kimg_init_end()
{
    return (ptr)_init_sect_end;
}

size_t kimg_map_offset()
{
    return (ptr)_kernel_map_offset;
}

ptr kimg_paddr()
{
    return (ptr)_kernel_base;
}

ptr kimg_vaddr()
{
    return (ptr)_kernel_vaddr;
}

size_t kimg_size()
{
    return (ptr)_kernel_size;
}
