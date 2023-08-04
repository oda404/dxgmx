/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
*/

/* specify the entry point */
ENTRY(_start)

_kernel_vbase = 0xC0100000;
_kernel_pbase = 0x100000;

ASSERT(_kernel_vbase >= _kernel_pbase, "The kernel's virtual base can't be smaller the it's physical base");

_kernel_map_offset = _kernel_vbase - _kernel_pbase;

SECTIONS
{
    . = _kernel_vbase;

    .bootloader ALIGN(4K) : AT(ADDR(.bootloader) - _kernel_map_offset)
    {
        _bootloader_start = .;
        KEEP(*(.bootloader))
        _bootloader_end = .;
    }

    .text ALIGN(4K) : AT (ADDR(.text) - _kernel_map_offset)
    {
        _text_start = .;
        *(.text)
        _text_end = .;
    }

    .syscalls ALIGN(4K) : AT (ADDR(.syscalls) - _kernel_map_offset)
    {
        _syscalls_start = .;
        *(.syscalls)
        _syscalls_end = .;
    }

    .useraccess ALIGN(4K) : AT (ADDR(.useraccess) - _kernel_map_offset)
    {
        _useraccess_start = .;
        *(.useraccess)
        _useraccess_end = .;
    }

    .modules ALIGN(4K) : AT(ADDR(.modules) - _kernel_map_offset)
    {
        _modules_start = .;
        *(.modules)
        _modules_end = .;
    }

    .init ALIGN(4K) : AT (ADDR(.init) - _kernel_map_offset)
    {
        _init_start = .;
        *(.init*)        
        _init_end = .;
    }

    .data ALIGN(4K) : AT (ADDR(.data) - _kernel_map_offset)
    {
        _data_start = .;
        *(.data)
        _data_end = .;
    }

    .ro_post_init ALIGN(4K) : AT (ADDR(.ro_post_init) - _kernel_map_offset)
    {
        _ro_post_init_start = .;
        *(.ro_post_init)
        _ro_post_init_end = .;
    }

    .rodata ALIGN(4K) : AT (ADDR(.rodata) - _kernel_map_offset)
    {
        _rodata_start = .;
        *(.rodata*)
        _rodata_end = .;
    }

    /* The stack is placed here, so if it grows too large it will hit rodata, 
    and we'll catch that */
    .kstack ALIGN(4K) : AT (ADDR(.kstack) - _kernel_map_offset)
    {
        _kstack_start = .;
        *(.kstack)
        _kstack_end = .;
    }

    .bss ALIGN(4K) : AT (ADDR(.bss) - _kernel_map_offset)
    {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }

    .ksyms ALIGN(4K) : AT(ADDR(.ksyms) - _kernel_map_offset)
    {
        _ksyms_start = .;
        *(.ksyms)
        _ksyms_end = .;
    }

    /DISCARD/ :
    {
        *(.comment)
        *(.debug*)
    }

    . = ALIGN(4K);

    _kernel_size = . - _kernel_vbase;
}
