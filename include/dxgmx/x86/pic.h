/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_PIC8259_H
#define _DXGMX_X86_PIC8259_H

#include <dxgmx/types.h>

/* I m just going to assume there are exactly two PICs. */

void pic8259_remap(u8 master_offset, u8 slave_offset);
void pic8259_mask_irq_line(u8 irqline);
void pic8259_unmask_irq_line(u8 irqline);
/*
 * Sets the IRQ mask for given PIC
 * @param mask 8 bit mask
 * @param pic 0 for master, increment for slaves
 */
void pic8259_set_mask(u8 mask, u8 pic);
/*
 * In Service Register is a bit mask that
 * tells us which irq s have been sent to the CPU
 * and are being processed.
 * @param pic 0 for master, increment for slaves.
 */
u8 pic8259_get_isr(u8 pic);
/*
 * Interrupt Request Register is a bit mask that tells us which
 * interrupts have been raised, but not yet sent to the CPU.
 * They will only be sent to the CPU once it s done with
 * any interrupts it s currently handling and if they aren t masked.
 * @param pic 0 for master, increment for slaves.
 */
u8 pic8259_get_irr(u8 pic);
/*
 * Signals End Of Interrupt to the given PIC. Needs to be called
 * at the end of every interrupt handler.
 * @param pic 0 for master, increment for slaves.
 */
void pic8259_signal_eoi(u8 pic);
/*
 * Disables the given PIC.
 * @param pic 0 for master, increment for slaves.
 */
void pic8259_disable(u8 pic);
/*
 * Idk if there are devices with more than 2 PICs,
 * and I don t support single PIC machines so this function
 * returns 2 hardcoded.
 */
u8 pic8259_get_pics_count();
/**
 * Get the current irqmask from both PICs.
 */
u16 pic8259_get_mask();

#endif // _DXGMX_X86_PIC8259_H
