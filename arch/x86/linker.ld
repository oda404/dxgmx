/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

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
        _bootloader_section_base = .;
        KEEP(*(.bootloader))
        _bootloader_section_end = .;
    }

    .text ALIGN(4K) : AT (ADDR(.text) - _kernel_map_offset)
    {
        _text_section_base = .;
        *(.text)
        _text_section_end = .;
    }

    .rodata ALIGN(4K) : AT (ADDR(.rodata) - _kernel_map_offset)
    {
        _rodata_section_base = .;
        *(.rodata*)
        _rodata_section_end = .;
    }

    .ro_after_stage1 ALIGN(4K) : AT (ADDR(.ro_after_stage1) - _kernel_map_offset)
    {
        _ro_after_stage1_section_base = .;
        *(.ro_after_stage1)
        _ro_after_stage1_section_end = .;
    }

    .data ALIGN(4K) : AT (ADDR(.data) - _kernel_map_offset)
    {
        _data_section_base = .;
        *(.data)
        _data_section_end = .;
    }

    .bss ALIGN(4K) : AT (ADDR(.bss) - _kernel_map_offset)
    {
        _bss_section_base = .;
        *(COMMON)
        *(.bss)
        _bss_section_end = .;
    }

    .shstrtab ALIGN(4K) : AT (ADDR(.shstrtab) - _kernel_map_offset)
    {
        *(.shstrtab*)
    }

    .ksyms ALIGN(4K) : AT(ADDR(.ksyms) - _kernel_map_offset)
    {
        _ksyms_section_base = .;
        *(.ksyms)
        _ksyms_section_end = .;
    }

    .init ALIGN(4K) : AT (ADDR(.init) - _kernel_map_offset)
    {
        _init_section_base = .;
        *(.init*)        
        _init_section_end = .;
    }

    _kernel_size = . - _kernel_vaddr;
}
