/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

/* specify the entry point */
ENTRY(_start)

SECTIONS
{
    . = 1M; /* start putting sections at 1Mib */
    _kernel_base = .;

    .text ALIGN(4K) : /* text section */
    {
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata ALIGN(4K) : /* read only section */
    {
        *(.rodata)
    }

    .data ALIGN(4K) : /* r/w section */
    {
        *(.data)
    }

    .bss ALIGN(4K) :  /* initial stack */
    {
        *(COMMON)
        *(.bss)
    }
    _kernel_end = .;
}
