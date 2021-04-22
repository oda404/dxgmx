/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_PIC_H__
#define __DXGMX_PIC_H__

#include<stdint.h>

/* i m just going to assume there are exactly 2 PICs */
void pic_remap(uint8_t master_offset, uint8_t slave_offset);
/* a masked irq will not be sent to the CPU */
void pic_mask_irq(uint8_t irqline);
void pic_unmask_irq(uint8_t irqline);
/* 
 * In Service Register is a bit mask that 
 * tells us which irq s have been sent to the CPU
 * and are being processed.
 * @param pic 0 for master, increment for slaves.
*/
uint8_t pic_get_isr(uint8_t pic);
/* 
 * Interrupt Request Register is a bit mask that tells us which 
 * interrupts have been raised, but not yet sent to the CPU.
 * They will only be sent to the CPU once it s done with 
 * any interrupts it s currently handling and if they aren t masked.
 * @param pic 0 for master, increment for slaves.
*/
uint8_t pic_get_irr(uint8_t pic);
void pic_disable();

#endif // __DXGMX_PIC_H__
