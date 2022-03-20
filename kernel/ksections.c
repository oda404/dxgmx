
#include <dxgmx/ksections.h>
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

ptr ksections_get_text_start()
{
    return (ptr)_text_sect_start;
}

ptr ksections_get_text_end()
{
    return (ptr)_text_sect_end;
}

ptr ksections_get_kstack_top()
{
    return (ptr)_kernel_stack_top;
}

ptr ksections_get_kstack_bot()
{
    return (ptr)_kernel_stack_bot;
}

ptr ksections_get_init_start()
{
    return (ptr)_init_sect_start;
}

ptr ksections_get_init_end()
{
    return (ptr)_init_sect_end;
}
