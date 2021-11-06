/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_IDT_H
#define _DXGMX_X86_IDT_H

#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/compiler_attrs.h>
#include<dxgmx/types.h>

#define TRAP0  0x0
#define TRAP1  0x1
#define TRAP2  0x2
#define TRAP3  0x3
#define TRAP4  0x4
#define TRAP5  0x5
#define TRAP6  0x6
#define TRAP7  0x7
#define TRAP8  0x8
#define TRAP9  0x9
#define TRAP10 0xA
#define TRAP11 0xB
#define TRAP12 0xC
#define TRAP13 0xD
#define TRAP14 0xE
#define TRAP15 0xF
#define TRAP16 0x10
#define TRAP17 0x11
#define TRAP18 0x12
#define TRAP19 0x13
#define TRAP20 0x14
#define TRAP21 0x15
#define TRAP22 0x16
#define TRAP23 0x17
#define TRAP24 0x18
#define TRAP25 0x19
#define TRAP26 0x1A
#define TRAP27 0x1B
#define TRAP28 0x1C
#define TRAP29 0x1D
#define TRAP30 0x1E
#define TRAP31 0x1F

#define IRQ0  0x20
#define IRQ1  0x21
#define IRQ2  0x22
#define IRQ3  0x23
#define IRQ4  0x24
#define IRQ5  0x25
#define IRQ6  0x26
#define IRQ7  0x27
#define IRQ8  0x28
#define IRQ9  0x29
#define IRQ10 0x2A
#define IRQ11 0x2B
#define IRQ12 0x2C
#define IRQ13 0x2D
#define IRQ14 0x2E
#define IRQ15 0x2F

#ifdef _X86_

typedef struct
_ATTR_PACKED S_IDTEntry
{
    /* First half of the isr handler address. */
    u16 base_0_15;
    /* Code selector. (kernel's code selector). */
    u16 selector;
    /* Must be 0. */
    u8  unused;
    /* The type of the gate. Also contains the storage segment bit. Set using IDT_GATE_TYPE_* macros. */
    u8  type: 5;
    /* The privilege level of the descriptor pointed to by the 'selector' field. Set using IDT_DESC_PRIV_* macros. */
    u8  privilege: 2;
    /* If the interrupt is unused, set to 0. */
    u8  present: 1;
    /* Second half of the isr handler address. */
    u16 base_16_31;
} IDTEntry;

typedef struct 
_ATTR_PACKED S_IDTR
{
    u16 limit;
    IDTEntry *base;
} IDTR;

#endif // _X86_

typedef void (*isr)(const InterruptFrame *frame, const void *data);
/* The ISR trashcan is an uint32 value that gets incremented every time an interrupt without a set ISR gets fired.
Why does it exist ? idk. */
u32 idt_get_isr_trashcan();
void idt_init();
/* Registers 'cb' as the ISR to be used for the interrupt 'irq' */
bool idt_register_isr(u8 irq, isr cb);

#endif // _DXGMX_X86_IDT_H
