/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_IDT_H
#define _DXGMX_X86_IDT_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/interrupt_frame.h>

#define TRAP_DIV_ERR 0x0
#define TRAP_DEBUG 0x1
#define TRAP_NMI 0x2
#define TRAP_BREAKPOINT 0x3
#define TRAP_OVERFLOW 0x4
#define TRAP_BOUND_EXCEEDED 0x5
#define TRAP_INVALID_OPCODE 0x6
#define TRAP_FPU_NOT_AVAIL 0x7
#define TRAP_DOUBLEFAULT 0x8
#define TRAP_INVALID_TSS 0xA
#define TRAP_ABSENT_SEGMENT 0xB
#define TRAP_SSFAULT 0xC
#define TRAP_GPF 0xD
#define TRAP_PAGEFAULT 0xE
// #define TRAP_RESERVED 0xF
#define TRAP_X87_FP_EXCEPTION 0x10
#define TRAP_ALIGNMENT_CHECK 0x11
#define TRAP_MACHINE_CHECK 0x12
#define TRAP_SIMD_FP_EXCEPTION 0x13
#define TRAP_VIRT_EXCEPTION 0x14
/* "A control protection exception is triggered when a control flow transfer
attempt violated shadow stack or indirect branch tracking constraints.
For example, the return address for a RET instruction differs from the
safe copy on the shadow stack; or a JMP instruction arrives at a non-
ENDBR instruction." - from a linux patch. */
#define TRAP_CONTROL_PROTECTION_EXCEPTION 0x15
// #define TRAP_RESERVED 0x16
// #define TRAP_RESERVED 0x17
// #define TRAP_RESERVED 0x18
// #define TRAP_RESERVED 0x19
// #define TRAP_RESERVED 0x1A
// #define TRAP_RESERVED 0x1B
#define TRAP_HYPERVISOR_EXCEPTION 0x1C
#define TRAP_VMM_COMM_EXCEPTION 0x1D
#define TRAP_SECURITY_EXCEPTION 0x1E
// #define TRAP_RESERVED 0x1F

#define IRQ_PIT 0x20
#define IRQ_PS2KBD 0x21
#define IRQ_COM2 0x23
#define IRQ_COM1 0x24
#define IRQ_LPT 0x25
#define IRQ_FLOPPY 0x26
#define IRQ_LPT1 0x27
#define IRQ_RTC 0x28
#define IRQ_ISA9 0x29
#define IRQ_ISA10 0x2A
#define IRQ_ISA11 0x2B
#define IRQ_PS2MOUSE 0x2C
#define IRQ_FPU 0x2D
#define IRQ_ATA1 0x2E
#define IRQ_ATA2 0x2F

#ifdef CONFIG_X86

typedef struct _ATTR_PACKED S_IDTEntry
{
    /* First half of the isr handler address. */
    u16 base_0_15;
    /* Code selector. (kernel's code selector). */
    u16 selector;
    /* Must be 0. */
    u8 unused;
    /* The type of the gate. Also contains the storage segment bit. Set using
     * IDT_GATE_TYPE_* macros. */
    u8 type : 5;
    /* The privilege level of the descriptor pointed to by the 'selector' field.
     * Set using IDT_DESC_PRIV_* macros. */
    u8 privilege : 2;
    /* If the interrupt is unused, set to 0. */
    u8 present : 1;
    /* Second half of the isr handler address. */
    u16 base_16_31;
} IDTEntry;

typedef struct _ATTR_PACKED S_IDTR
{
    u16 limit;
    IDTEntry* base;
} IDTR;

#endif // CONFIG_X86

typedef void (*x86isr_t)(InterruptFrame* frame);

void idt_init();

/**
 *  Register an x86 ISR for a trap.
 *
 * 'n' The interrupt number.
 * 'ring' The minimum ring that can call this interrupt. 0 or 3.
 * 'cb' The ISR.
 *
 * Returns:
 * 0 on success.
 * -1 on invalid ring.
 */
int idt_register_trap_isr(intn_t n, u8 ring, x86isr_t cb);

/**
 * Register an x86 ISR for an IRQ. Note that you can also register a platform
 * agnostic ISR for an IRQ using interrupts_register_irq_isr, which will not
 * have the InterruptFrame* as a parameter.
 *
 * 'n' The interrupt number.
 * 'cb' The ISR.
 *
 * Returns:
 * 0 on success.
 */
int idt_register_irq_isr(intn_t n, x86isr_t cb);

#endif // _DXGMX_X86_IDT_H
