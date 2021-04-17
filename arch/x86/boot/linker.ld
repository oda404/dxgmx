

/* specify the entry point */
ENTRY(_start)

SECTIONS
{
    . = 1M; /* start putting sections at 1Mib */
    kernel_addr_base = .;

    .text BLOCK(4K) : ALIGN(4K) /* text section */
    {
        KEEP(*(.multiboot))
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) /* read only section */
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) /* r/w section */
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) /* initial stack */
    {
        *(COMMON)
        *(.bss)
    }
    kernel_addr_end = .;
}
