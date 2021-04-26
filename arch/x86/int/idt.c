/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/gcc/attrs.h>
#include<stdint.h>

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
typedef struct
__ATTR_PACKED
{
    uint16_t base_0_15;
    uint16_t selector;
    uint8_t  unused;
    uint8_t  type;
    uint16_t base_16_31;
} IDTEntry_x86;

typedef struct
__ATTR_PACKED
{
    uint16_t size;
    uint32_t base;
} IDTR;

#ifdef __X86__
static IDTEntry_x86 idt[255];
#endif // __X86__

/* handy macros for irq creation */

#define IDT_INT_GATE_TYPE_TASK_32 0b10101
#define IDT_INT_GATE_TYPE_INT_16  0b00110
#define IDT_INT_GATE_TYPE_TRAP_16 0b00111
#define IDT_INT_GATE_TYPE_INT_32  0b01110
#define IDT_INT_GATE_TYPE_TRAP_32 0b01111

#define IDT_INT_DESC_PRIV_LVL_0   (0 << 5)
#define IDT_INT_DESC_PRIV_LVL_1   (1 << 5)
#define IDT_INT_DESC_PRIV_LVL_2   (2 << 5)
#define IDT_INT_DESC_PRIV_LVL_3   (3 << 5)

#define IDT_INT_PRESENT           (1 << 7)

/* irq entry points */
extern int irq0_entry();
extern int irq1_entry();
extern int irq2_entry();
extern int irq3_entry();
extern int irq4_entry();
extern int irq5_entry();
extern int irq6_entry();
extern int irq7_entry();
extern int irq8_entry();
extern int irq9_entry();
extern int irq10_entry();
extern int irq11_entry();
extern int irq12_entry();
extern int irq13_entry();
extern int irq14_entry();
extern int irq15_entry();

#define IRQ_ENTRYPOINTS_CNT 16
typedef int (*irq_entrypoint)(void);
/* create an array out of all the irq entry points for iterating */
static irq_entrypoint irq_entrypoints[IRQ_ENTRYPOINTS_CNT] = {
    &irq0_entry,
    &irq1_entry,
    &irq2_entry,
    &irq3_entry,
    &irq4_entry,
    &irq5_entry,
    &irq6_entry,
    &irq7_entry,
    &irq8_entry,
    &irq9_entry,
    &irq10_entry,
    &irq11_entry,
    &irq12_entry,
    &irq13_entry,
    &irq14_entry,
    &irq15_entry
};

extern void load_idtr(IDTR *idtr);

static void idt_x86_encode_entry(
    uint32_t base,
    uint16_t selector,
    uint8_t  type,
    IDTEntry_x86 *target
)
{
    target->base_0_15  = (base & 0xFFFF);
    target->base_16_31 = (base & 0xFFFF0000) >> 16;
    target->selector   = selector;
    target->type       = type;
}

/*
* Intel reserves the first 32(0-31) interrupts for exceptions
* So we remap the master and slave PIC.
* Master will have offset 32, right after the intel shit.
* Slave will have offset 40, right after the master.
*/
#define MASTER_PIC_OFFSET 32
#define SLAVE_PIC_OFFSET  (MASTER_PIC_OFFSET + 8)

void idt_init()
{
    pic8259_remap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
    /* paranoia */
    asm volatile("cli");

    /* ISA interrupts */
    for(uint8_t i = 0; i < 16; ++i)
    {
        uint32_t irq_entry_base = (uint32_t)irq_entrypoints[i];

        idt_x86_encode_entry(
            irq_entry_base,
            0x8, 
            IDT_INT_GATE_TYPE_INT_32 | IDT_INT_DESC_PRIV_LVL_0 | IDT_INT_PRESENT,
            &idt[i + MASTER_PIC_OFFSET]
        );
    }

    IDTR idtr;
    idtr.size = sizeof(idt);
    idtr.base = (uint32_t)idt;

    load_idtr(&idtr);
}
