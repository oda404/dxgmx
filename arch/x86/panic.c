/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/panic.h>
#include <dxgmx/x86/idt.h>

_ATTR_COLD int panic_arch_prepare()
{
    __asm__ volatile("cli");

    // TODO: maybe register stubs for each ISR.

    return 0;
}
