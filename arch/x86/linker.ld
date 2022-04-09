/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

OUTPUT_FORMAT(elf32-i386)

/* specify the entry point */
ENTRY(_start)

_kernel_vbase = 0xC0100000;
_kernel_pbase = 0x100000;
_kernel_map_offset = _kernel_vbase - _kernel_pbase;

ASSERT(_kernel_vbase >= _kernel_pbase, "The kernel's virtual base can't be smaller the it's physical base");

SECTIONS
{
    . = 0xC0100000;
    _kernel_vaddr = .;
    _kernel_base = 0x100000;

    _kernel_map_offset = _kernel_vaddr - _kernel_base;

    .bootloader ALIGN(4K) : AT(ADDR(.bootloader) - _kernel_map_offset)
    {
        _bootloader_sect_start = .;
        KEEP(*(.bootloader))
        _bootloader_sect_end = .;
    }

    .text ALIGN(4K) : AT (ADDR(.text) - _kernel_map_offset)
    {
        _text_sect_start = .;
        *(.text)
        _text_sect_end = .;
    }

    .rodata ALIGN(4K) : AT (ADDR(.rodata) - _kernel_map_offset)
    {
        _rodata_sect_start = .;
        *(.rodata*)
        _rodata_sect_end = .;
    }

    .ro_post_init ALIGN(4K) : AT (ADDR(.ro_post_init) - _kernel_map_offset)
    {
        _ro_post_init_sect_start = .;
        *(.ro_post_init)
        _ro_post_init_sect_end = .;
    }

    .data ALIGN(4K) : AT (ADDR(.data) - _kernel_map_offset)
    {
        _data_sect_start = .;
        *(.data)
        _data_sect_end = .;
    }

    .bss ALIGN(4K) : AT (ADDR(.bss) - _kernel_map_offset)
    {
        _bss_sect_start = .;
        *(COMMON)
        *(.bss)
        _bss_sect_end = .;
    }

    .shstrtab ALIGN(4K) : AT (ADDR(.shstrtab) - _kernel_map_offset)
    {
        *(.shstrtab*)
    }

    .modules ALIGN(4K) : AT(ADDR(.modules) - _kernel_map_offset)
    {
        _modules_sect_start = .;
        *(.modules)
        _modules_sect_end = .;
    }

    .ksyms ALIGN(4K) : AT(ADDR(.ksyms) - _kernel_map_offset)
    {
        _ksyms_sect_start = .;
        *(.ksyms)
        _ksyms_sect_end = .;
    }

    .init ALIGN(4K) : AT (ADDR(.init) - _kernel_map_offset)
    {
        _init_sect_start = .;
        *(.init*)        
        _init_sect_end = .;
    }

    /DISCARD/ :
    {
        *(.comment)
    }

    _kernel_size = . - _kernel_vaddr;
}
