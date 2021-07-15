

#include<dxgmx/x86/sysidt.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/attrs.h>
#include<stdint.h>

#define ISR_ENTRY(id)                \
_ATTR_NAKED                          \
void isr##id() {                     \
    asm volatile(                    \
        "pusha                   \n" \
        "call isr" #id "_handler \n" \
        "popa                    \n" \
        "iret                    \n" \
    );                               \
}

#define IRQ_ENTRY(id)                \
_ATTR_NAKED                          \
void irq##id() {                     \
    asm volatile(                    \
        "pusha                   \n" \
        "call irq" #id "_handler \n" \
        "popa                    \n" \
        "iret                    \n" \
    );                               \
}

ISR_ENTRY(0)
ISR_ENTRY(1)
ISR_ENTRY(2)
ISR_ENTRY(3)
ISR_ENTRY(4)
ISR_ENTRY(5)
ISR_ENTRY(6)
ISR_ENTRY(7)
ISR_ENTRY(8)
ISR_ENTRY(9)
ISR_ENTRY(10)
ISR_ENTRY(11)
ISR_ENTRY(12)
ISR_ENTRY(13)
ISR_ENTRY(14)
ISR_ENTRY(15)
ISR_ENTRY(16)
ISR_ENTRY(17)
ISR_ENTRY(18)
ISR_ENTRY(19)
ISR_ENTRY(20)
ISR_ENTRY(21)
ISR_ENTRY(22)
ISR_ENTRY(23)
ISR_ENTRY(24)
ISR_ENTRY(25)
ISR_ENTRY(26)
ISR_ENTRY(27)
ISR_ENTRY(28)
ISR_ENTRY(29)
ISR_ENTRY(30)
ISR_ENTRY(31)

IRQ_ENTRY(0)
IRQ_ENTRY(1)
IRQ_ENTRY(2)
IRQ_ENTRY(3)
IRQ_ENTRY(4)
IRQ_ENTRY(5)
IRQ_ENTRY(6)
IRQ_ENTRY(7)
IRQ_ENTRY(8)
IRQ_ENTRY(9)
IRQ_ENTRY(10)
IRQ_ENTRY(11)
IRQ_ENTRY(12)
IRQ_ENTRY(13)
IRQ_ENTRY(14)
IRQ_ENTRY(15)

typedef void (*isr_entrypoint)(void);
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

void sysidt_init()
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

    idtr.base = idt;
    idtr.limit = sizeof(idt) - 1;

    pic8259_remap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
    idt_load(&idtr);

    asm volatile("sti");
}

void isr0_handler()
{

}

void isr1_handler()
{

}

void isr2_handler()
{

}

void isr3_handler()
{

}

void isr4_handler()
{

}

void isr5_handler()
{

}

void isr6_handler()
{

}

void isr7_handler()
{

}

void isr8_handler()
{

}

void isr9_handler()
{

}

void isr10_handler()
{

}

void isr11_handler()
{

}

void isr12_handler()
{

}

void isr13_handler()
{

}

void isr14_handler()
{

}

void isr15_handler()
{

}

void isr16_handler()
{

}

void isr17_handler()
{

}

void isr18_handler()
{

}

void isr19_handler()
{

}

void isr20_handler()
{

}

void isr21_handler()
{

}

void isr22_handler()
{

}

void isr23_handler()
{

}

void isr24_handler()
{

}

void isr25_handler()
{

}

void isr26_handler()
{

}

void isr27_handler()
{

}

void isr28_handler()
{

}

void isr29_handler()
{

}

void isr30_handler()
{

}

void isr31_handler()
{

}

/* PIT */
void irq0_handler()
{
    pic8259_signal_eoi(0);
}

/* keyboard */
void irq1_handler()
{
    unsigned char scan_code = port_inb(0x60); 
    kprintf("%d ", scan_code);
    pic8259_signal_eoi(0);
}

/* used internally by the 2 PICs */
void irq2_handler()
{
    pic8259_signal_eoi(0);
}

/* COM2 */
void irq3_handler()
{
    pic8259_signal_eoi(0);
}

/* COM1 */
void irq4_handler()
{
    pic8259_signal_eoi(0);
}

/* LPT2 */
void irq5_handler()
{
    pic8259_signal_eoi(0);
}

/* floppy disk */
void irq6_handler()
{
    pic8259_signal_eoi(0);
}

/* LPT1 */
void irq7_handler()
{
    pic8259_signal_eoi(0);
}

/* CMOS RTC */
void irq8_handler()
{
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq9_handler()
{
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq10_handler()
{
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq11_handler()
{
    pic8259_signal_eoi(1);
}

/* PS2 mouse */
void irq12_handler()
{
    pic8259_signal_eoi(1);
}

/* FPU/co-CPU/inter-CPU */
void irq13_handler()
{
    pic8259_signal_eoi(1);
}

/* primary ATA hard disk */
void irq14_handler()
{
    pic8259_signal_eoi(1);
}

/* secondary ATA hard disk */
void irq15_handler()
{
    pic8259_signal_eoi(1);
}