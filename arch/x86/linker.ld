/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

/* specify the entry point */
ENTRY(_start)

SECTIONS
{
    . = 1M; /* start putting sections at 1MiB */
    _kernel_base = .;

    .bootloader ALIGN(4K) : /* bootloader specific section */
    {
        _bootloader_section_base = .;
        KEEP(*(.bootloader))
        _bootloader_section_end = .;
    }

    .text ALIGN(4K) : /* text section */
    {
        _text_section_base = .;
        *(.text)
        _text_section_end = .;
    }

    .rodata ALIGN(4K) : /* read only section */
    {
        _rodata_section_base = .;
        *(.rodata*)
        _rodata_section_end = .;
    }

    .data ALIGN(4K) : /* r/w section */
    {
        _data_section_base = .;
        *(.data)
        _data_section_end = .;
    }

    .bss ALIGN(4K) :  /* initial stack */
    {
        _bss_section_base = .;
        *(COMMON)
        *(.bss)
        _bss_section_end = .;
    }

    .shstrtab ALIGN(4K): 
    {
        *(.shstrtab*)
    }

    .ro_after_stage1 ALIGN(4K) :
    {
        _ro_after_stage1_section_base = .;
        *(.ro_after_stage1)
        _ro_after_stage1_section_end = .;
    }

    .ksyms ALIGN(4K) :
    {
        _ksyms_section_base = .;
        *(.ksyms)
        _ksyms_section_end = .;
    }

    .init ALIGN(4K) :
    {
        _init_section_base = .;
        *(.init)
        _init_section_end = .;
    }
    _kernel_end = .;
}
