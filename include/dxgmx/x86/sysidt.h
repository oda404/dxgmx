
#ifndef _DXMGX_X86_INTERRUPTS_H
#define _DXMGX_X86_INTERRUPTS_H

#include<dxgmx/x86/interrupt_frame.h>

#define ISR0  0x0
#define ISR1  0x1
#define ISR2  0x2
#define ISR3  0x3
#define ISR4  0x4
#define ISR5  0x5
#define ISR6  0x6
#define ISR7  0x7
#define ISR8  0x8
#define ISR9  0x9
#define ISR10 0xA
#define ISR11 0xB
#define ISR12 0xC
#define ISR13 0xD
#define ISR14 0xE
#define ISR15 0xF
#define ISR16 0x10
#define ISR17 0x11
#define ISR18 0x12
#define ISR19 0x13
#define ISR20 0x14
#define ISR21 0x15
#define ISR22 0x16
#define ISR23 0x17
#define ISR24 0x18
#define ISR25 0x19
#define ISR26 0x1A
#define ISR27 0x1B
#define ISR28 0x1C
#define ISR29 0x1D
#define ISR30 0x1E
#define ISR31 0x1F

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

typedef void (*interrupt_callback)(const InterruptFrame *frame, const void *data);

void sysidt_init();
int sysidt_register_callback(uint8_t interrupt_n, interrupt_callback callback);

#endif //_DXMGX_X86_INTERRUPTS_H
