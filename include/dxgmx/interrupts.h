/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_INTERRUPTS_H
#define _DXGMX_INTERRUPTS_H

#include <dxgmx/types.h>

/**
 * A few notes about the terminology used here:
 * A trap is a software interrupt/exception.
 * An IRQ is a hardware interrupt.
 * An interrupt is any type of interrupt (trap/IRQ/NMI).
 * An ISR (Interrupt Service Routine) is the handler of an interrupt.
 */

/* Vector number of a interrupt */
typedef u8 intn_t;

typedef void (*isr_t)();

/* Disable all hardware interrupts. */
void interrupts_disable_irqs();

/* Enable all hardware interrupts. */
void interrupts_enable_irqs();

/**
 * Register an ISR for an IRQ.
 *
 * 'n' The interrupt number.
 * 'isr' The ISR.
 *
 * Returns:
 * 0 on success.
 */
int interrupts_reqister_irq_isr(intn_t n, isr_t isr);

/**
 * Register an ISR for an trap.
 *
 * 'n' The interrupt number.
 * 'ring' The privilege level this trap can be called from.
 * 'isr' The ISR.
 *
 * Returns:
 * 0 on success.
 */
int interrupts_reqister_trap_isr(intn_t n, u8 ring, isr_t isr);

/**
 * Should be sent by a driver at the end of a hardware interrupt.
 * If not sent, no new interrupts will be received, and along with them the
 * whole system will probably freeze up.
 */
void interrupts_irq_done();

#endif // !_DXGMX_INTERRUPTS_H
