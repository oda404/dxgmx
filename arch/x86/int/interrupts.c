/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/pic.h>

_ATTR_ALWAYS_INLINE void interrupts_disable_irqs()
{
    __asm__ volatile("cli");
}

_ATTR_ALWAYS_INLINE void interrupts_enable_irqs()
{
    __asm__ volatile("sti");
}

int interrupts_reqister_irq_isr(intn_t n, isr_t isr)
{
    /* The only difference between x86isr_t and an isr_t is that x86isr_t has
     * an InterruptFrame* as a parameter, which is x86 specific */
    return idt_register_irq_isr(n, (x86isr_t)isr);
}

int interrupts_reqister_trap_isr(intn_t n, u8 ring, isr_t isr)
{
    /* The only difference between x86isr_t and an isr_t is that x86isr_t has
     * an InterruptFrame* as a parameter, which is x86 specific */
    return idt_register_trap_isr(n, ring, (x86isr_t)isr);
}

void interrupts_irq_done()
{
    /* I'm assuming only one bit will be set a time. */
    u8 isr = pic8259_get_isr(1);
    if (isr)
        pic8259_signal_eoi(1);

    pic8259_signal_eoi(0);
}
