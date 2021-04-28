/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_PIC_H__
#define __DXGMX_PIC_H__

#include<stdint.h>

/* I m just going to assume there are exactly two PICs. */

void pic8259_remap(uint8_t master_offset, uint8_t slave_offset);
void pic8259_mask_irq_line(uint8_t irqline);
void pic8259_unmask_irq_line(uint8_t irqline);
/*
 * Sets the IRQ mask for given PIC
 * @param mask 8 bit mask
 * @param pic 0 for master, increment for slaves
 */
void pic8259_set_mask(uint8_t mask, uint8_t pic);
/* 
 * In Service Register is a bit mask that 
 * tells us which irq s have been sent to the CPU
 * and are being processed.
 * @param pic 0 for master, increment for slaves.
 */
uint8_t pic8259_get_isr(uint8_t pic);
/* 
 * Interrupt Request Register is a bit mask that tells us which 
 * interrupts have been raised, but not yet sent to the CPU.
 * They will only be sent to the CPU once it s done with 
 * any interrupts it s currently handling and if they aren t masked.
 * @param pic 0 for master, increment for slaves.
 */
uint8_t pic8259_get_irr(uint8_t pic);
/*
 * Signals End Of Interrupt to the given PIC. Needs to be called
 * at the end of every interrupt handler.
 * @param pic 0 for master, increment for slaves.
 */
void pic8259_signal_eoi(uint8_t pic);
/*
 * Disables the given PIC.
 * @param pic 0 for master, increment for slaves.
 */
void pic8259_disable(uint8_t pic);
/*
 * Idk if there are devices with more than 2 PICs,
 * and I don t support single PIC machines so this function
 * returns 2 hardcoded.
 */
uint8_t pic8259_get_pics_count();

#endif // __DXGMX_PIC_H__
