

#include<dxgmx/x86/int/interrupts.h>
#include<dxgmx/x86/int/idt.h>
#include<dxgmx/x86/int/pic.h>
#include<stdint.h>

extern int isr0();
extern int isr1();
extern int isr2();
extern int isr3();
extern int isr4();
extern int isr5();
extern int isr6();
extern int isr7();
extern int isr8();
extern int isr9();
extern int isr10();
extern int isr11();
extern int isr12();
extern int isr13();
extern int isr14();
extern int isr15();
extern int isr16();
extern int isr17();
extern int isr18();
extern int isr19();
extern int isr20();
extern int isr21();
extern int isr22();
extern int isr23();
extern int isr24();
extern int isr25();
extern int isr26();
extern int isr27();
extern int isr28();
extern int isr29();
extern int isr30();
extern int isr31();

/* irq entry points */
extern int irq0();
extern int irq1();
extern int irq2();
extern int irq3();
extern int irq4();
extern int irq5();
extern int irq6();
extern int irq7();
extern int irq8();
extern int irq9();
extern int irq10();
extern int irq11();
extern int irq12();
extern int irq13();
extern int irq14();
extern int irq15();

typedef int (*isr_entrypoint)(void);
static isr_entrypoint isrs[] = {
    &isr0,
    &isr1,
    &isr2,
    &isr3,
    &isr4,
    &isr5,
    &isr6,
    &isr7,
    &isr8,
    &isr9,
    &isr10,
    &isr11,
    &isr12,
    &isr13,
    &isr14,
    &isr15,
    &isr16,
    &isr17,
    &isr18,
    &isr19,
    &isr20,
    &isr21,
    &isr22,
    &isr23,
    &isr24,
    &isr25,
    &isr26,
    &isr27,
    &isr28,
    &isr29,
    &isr30,
    &isr31
};
static isr_entrypoint irqs[] = {
    &irq0,
    &irq1,
    &irq2,
    &irq3,
    &irq4,
    &irq5,
    &irq6,
    &irq7,
    &irq8,
    &irq9,
    &irq10,
    &irq11,
    &irq12,
    &irq13,
    &irq14,
    &irq15
};

static IDTEntry idt[256];
static IDTR idtr;

/*
* Intel reserves the first 32(0-31) interrupts for exceptions
* So we remap the master and slave PIC.
* Master will have offset 32, right after the intel shit.
* Slave will have offset 40, right after the master.
*/
#define MASTER_PIC_OFFSET 32
#define SLAVE_PIC_OFFSET  (MASTER_PIC_OFFSET + 8)

void interrupts_init()
{
    asm volatile("cli");

    for(uint8_t i = 0; i < 32; ++i)
    {
        idt_encode_entry(
            (uint32_t)isrs[i],
            0x8,
            IDT_GATE_TYPE_INT_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT,
            &idt[i]
        );
    }

    /* ISA interrupts */
    for(uint8_t i = 0; i < 16; ++i)
    {
        idt_encode_entry(
            (uint32_t)irqs[i],
            0x8, 
            IDT_GATE_TYPE_INT_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT,
            &idt[i + MASTER_PIC_OFFSET]
        );
    }

    idtr.size = sizeof(idt) - 1;
    idtr.base = (uint32_t)idt;

    pic8259_remap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
    asm volatile("lidt (%0); sti;": : "b"(&idtr));
}
