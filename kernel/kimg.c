/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kimg.h>

extern u8 _kernel_stack_top[];
extern u8 _kernel_stack_bot[];
extern u8 _kernel_pbase[];
extern u8 _kernel_vbase[];
extern u8 _text_start[];
extern u8 _text_end[];
extern u8 _rodata_start[];
extern u8 _rodata_end[];
extern u8 _ro_post_init_start[];
extern u8 _ro_post_init_end[];
extern u8 _init_start[];
extern u8 _init_end[];
extern u8 _data_start[];
extern u8 _data_end[];
extern u8 _bss_start[];
extern u8 _bss_end[];
extern u8 _bootloader_start[];
extern u8 _bootloader_end[];
extern u8 _kinit_stage3_text_start[];
extern u8 _kinit_stage3_text_end[];
extern u8 _kernel_map_offset[];
extern u8 _kernel_size[];
extern u8 _modules_start[];
extern u8 _modules_end[];
extern u8 _ksyms_start[];
extern u8 _ksyms_end[];
extern u8 _useraccess_start[];
extern u8 _useraccess_end[];
extern u8 _syscalls_start[];
extern u8 _syscalls_end[];

ptr kimg_bootloader_start()
{
    return (ptr)_bootloader_start;
}

ptr kimg_bootloader_end()
{
    return (ptr)_bootloader_end;
}

ptr kimg_text_start()
{
    return (ptr)_text_start;
}

ptr kimg_text_end()
{
    return (ptr)_text_end;
}

ptr kimg_kstack_top()
{
    return (ptr)_kernel_stack_top;
}

ptr kimg_kstack_bot()
{
    return (ptr)_kernel_stack_bot;
}

ptr kimg_bss_start()
{
    return (ptr)_bss_start;
}
ptr kimg_bss_end()
{
    return (ptr)_bss_end;
}

ptr kimg_data_start()
{
    return (ptr)_data_start;
}

ptr kimg_data_end()
{
    return (ptr)_data_end;
}

ptr kimg_rodata_start()
{
    return (ptr)_rodata_start;
}

ptr kimg_rodata_end()
{
    return (ptr)_rodata_end;
}

ptr kimg_ro_postinit_start()
{
    return (ptr)_ro_post_init_start;
}

ptr kimg_ro_postinit_end()
{
    return (ptr)_ro_post_init_end;
}

ptr kimg_init_start()
{
    return (ptr)_init_start;
}

ptr kimg_init_end()
{
    return (ptr)_init_end;
}

ptr kimg_module_start()
{
    return (ptr)_modules_start;
}

ptr kimg_module_end()
{
    return (ptr)_modules_end;
}

ptr kimg_ksyms_start()
{
    return (ptr)_ksyms_start;
}

ptr kimg_ksyms_end()
{
    return (ptr)_ksyms_end;
}

size_t kimg_map_offset()
{
    return (ptr)_kernel_map_offset;
}

ptr kimg_paddr()
{
    return (ptr)_kernel_pbase;
}

ptr kimg_vaddr()
{
    return (ptr)_kernel_vbase;
}

size_t kimg_size()
{
    return (ptr)_kernel_size;
}

ptr kimg_useraccess_start()
{
    return (ptr)_useraccess_start;
}

ptr kimg_useraccess_end()
{
    return (ptr)_useraccess_end;
}

ptr kimg_syscalls_start()
{
    return (ptr)_syscalls_start;
}
ptr kimg_syscalls_end()
{
    return (ptr)_syscalls_end;
}
