/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_IDT_H
#define _DXGMX_X86_IDT_H

#include<dxgmx/attrs.h>
#include<stdint.h>

#define IDT_GATE_TYPE_TASK_32 0b10101
#define IDT_GATE_TYPE_INT_16  0b00110
#define IDT_GATE_TYPE_TRAP_16 0b00111
#define IDT_GATE_TYPE_INT_32  0b01110
#define IDT_GATE_TYPE_TRAP_32 0b01111

#define IDT_DESC_PRIV_0       (0 << 5)
#define IDT_DESC_PRIV_1       (1 << 5)
#define IDT_DESC_PRIV_2       (2 << 5)
#define IDT_DESC_PRIV_3       (3 << 5)

#define IDT_INT_PRESENT       (1 << 7)

/*
 * Format of a x86 IDT entry - 64 bits wide
 * 
 * |63                              48|47            40|39            32|
 * |----------------------------------+----------------+----------------|
 * |            base 16-31            |      type      |   unused (0)   |
 * |----------------------------------+----------------+----------------|
 * |            selector 16-31        |           base 0-15             |
 * |----------------------------------+---------------------------------|
 * |31                              16|15                              0|
*/

#ifdef _X86_

typedef struct
_ATTR_PACKED S_IDTEntry
{
    /* First half of the isr handler address. */
    uint16_t base_0_15;
    /* Code selector. (kernel's code selector). */
    uint16_t selector;
    /* Must be 0. */
    uint8_t  unused;
    /* The type of the gate. Also contains the storage segment bit. Set using IDT_GATE_TYPE_* macros. */
    uint8_t  type: 5;
    /* The privilege level of the descriptor pointed to by the 'selector' field. Set using IDT_DESC_PRIV_* macros. */
    uint8_t  privilege: 2;
    /* If the interrupt is unused, set to 0. */
    uint8_t  present: 1;
    /* Second half of the isr handler address. */
    uint16_t base_16_31;
} IDTEntry;

typedef struct 
_ATTR_PACKED S_IDTR
{
    uint16_t limit;
    IDTEntry *base;
} IDTR;

#endif // _X86_

void idt_encode_entry(
#ifdef _X86_
    uint32_t base,
#endif //_X86_
    uint16_t selector,
    uint8_t  flags,
    IDTEntry *entry
);

void idt_load(const IDTR *idtr);

#endif // _DXGMX_X86_IDT_H
