/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/x86/pic.h>

_ATTR_ALWAYS_INLINE void interrupts_disable_irqs()
{
    __asm__ volatile("cli");
}

_ATTR_ALWAYS_INLINE void interrupts_enable_irqs()
{
    __asm__ volatile("sti");
}

void interrupts_irq_done()
{
    /* I'm assuming only one bit will be set a time. */
    u8 isr = pic8259_get_isr(1);
    if (isr)
        pic8259_signal_eoi(1);

    pic8259_signal_eoi(0);
}
