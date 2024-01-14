/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/x86/gdt.h>
#include <dxgmx/x86/idt.h>

void kinit_arch()
{
    gdt_finish_init();
    /* Interrupts are still disabled, set them up now */
    idt_init();
}
