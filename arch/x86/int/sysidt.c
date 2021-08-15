/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/sysidt.h>
#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/attrs.h>
#include<dxgmx/string.h>
#include<stdint.h>

asm(
    ".type isr_exit, @function                       \n"
    "isr_exit:                                       \n"
    "  addl $4, %esp # jump over the InterruptFrame* \n"
    "  popa                                          \n"
    "  addl $4, %esp # jump over the code            \n"
    "  iretl                                         \n"
);

/* ISR entry that has the status code pushed on to the stack. */
#define ISR_ENTRY_CODE(id)                                     \
_ATTR_NAKED void isr##id() {                                   \
    asm volatile(                                              \
        "pusha                                             \n" \
        "pushl %esp              # set the InterruptFrame* \n" \
        "call  isr"#id"_handler                            \n" \
        "jmp isr_exit                                      \n" \
    );                                                         \
}

/* ISR entry that pushes a dummy status code on to the stack. */
#define ISR_ENTRY_NO_CODE(id)                                 \
_ATTR_NAKED void isr##id() {                                  \
    asm volatile(                                             \
        "pushl $0                #push a fake code        \n" \
        "pusha                                            \n" \
        "pushl %esp              #set the InterruptFrame* \n" \
        "call  isr"#id"_handler                           \n" \
        "jmp isr_exit                                     \n" \
    );                                                        \
}

#define IRQ_ENTRY(id)                 \
_ATTR_NAKED void irq##id() {          \
    asm volatile(                     \
        "pushl $0                 \n" \
        "pusha                    \n" \
        "pushl %esp               \n" \
        "call irq"#id"_handler    \n" \
        "jmp isr_exit             \n" \
    );                                \
}

ISR_ENTRY_NO_CODE(0)
ISR_ENTRY_NO_CODE(1)
ISR_ENTRY_NO_CODE(2)
ISR_ENTRY_NO_CODE(3)
ISR_ENTRY_NO_CODE(4)
ISR_ENTRY_NO_CODE(5)
ISR_ENTRY_NO_CODE(6)
ISR_ENTRY_NO_CODE(7)
ISR_ENTRY_CODE(8)
ISR_ENTRY_NO_CODE(9)
ISR_ENTRY_CODE(10)
ISR_ENTRY_CODE(11)
ISR_ENTRY_CODE(12)
ISR_ENTRY_CODE(13)
ISR_ENTRY_CODE(14)
ISR_ENTRY_NO_CODE(15)
ISR_ENTRY_NO_CODE(16)
ISR_ENTRY_CODE(17)
ISR_ENTRY_NO_CODE(18)
ISR_ENTRY_NO_CODE(19)
ISR_ENTRY_NO_CODE(20)
ISR_ENTRY_NO_CODE(21)
ISR_ENTRY_NO_CODE(22)
ISR_ENTRY_NO_CODE(23)
ISR_ENTRY_NO_CODE(24)
ISR_ENTRY_NO_CODE(25)
ISR_ENTRY_NO_CODE(26)
ISR_ENTRY_NO_CODE(27)
ISR_ENTRY_NO_CODE(28)
ISR_ENTRY_NO_CODE(29)
ISR_ENTRY_CODE(30)
ISR_ENTRY_NO_CODE(31)

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

typedef struct
S_InterruptCallbacks
{
#define CALLBACKS_MAX 8
    interrupt_callback callbacks[CALLBACKS_MAX];
    uint8_t            callbacks_count;
} InterruptCallbacks;

#define INTS_CALLBACKS_MAX 48
static InterruptCallbacks g_interrupts_callbacks[INTS_CALLBACKS_MAX];

void sysidt_init()
{
    interrupts_disable();

    for(uint8_t i = 0; i < 32; ++i)
    {
        idt_encode_entry(
            (uint32_t)isrs[i],
            SYSGDT_KERNEL_CS,
            IDT_GATE_TYPE_TRAP_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT,
            &idt[i]
        );
    }

    /* ISA interrupts */
    for(uint8_t i = 0; i < 16; ++i)
    {
        idt_encode_entry(
            (uint32_t)irqs[i],
            SYSGDT_KERNEL_CS, 
            IDT_GATE_TYPE_INT_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT,
            &idt[i + MASTER_PIC_OFFSET]
        );
    }

    idtr.base = idt;
    idtr.limit = sizeof(idt) - 1;

    pic8259_remap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
    idt_load(&idtr);

    interrupts_enable();
}

int sysidt_register_callback(uint8_t interrupt_n, interrupt_callback callback)
{
    if(interrupt_n >= INTS_CALLBACKS_MAX)
        return 1;

    InterruptCallbacks *cbs = &g_interrupts_callbacks[interrupt_n];
    if(cbs->callbacks_count >= CALLBACKS_MAX)
        return 2;
    
    cbs->callbacks[cbs->callbacks_count] = callback;
    return cbs->callbacks_count++;
}

void isr0_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr1_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr2_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr3_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr4_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr5_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr6_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr7_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr8_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr9_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr10_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr11_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr12_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr13_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr14_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr15_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr16_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr17_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr18_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr19_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr20_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr21_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr22_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr23_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr24_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr25_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr26_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr27_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr28_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr29_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr30_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

void isr31_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{

}

/* PIT */
void irq0_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* keyboard */
void irq1_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    unsigned char scan_code = port_inb(0x60); 
    kprintf("%d ", scan_code);
    pic8259_signal_eoi(0);
}

/* used internally by the 2 PICs */
void irq2_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* COM2 */
void irq3_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* COM1 */
void irq4_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* LPT2 */
void irq5_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* floppy disk */
void irq6_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* LPT1 */
void irq7_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(0);
}

/* CMOS RTC */
void irq8_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    const InterruptCallbacks *cbs = &g_interrupts_callbacks[IRQ8];
    for(size_t i = 0; i < cbs->callbacks_count; ++i)
        cbs->callbacks[i](frame, NULL);
    
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq9_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq10_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq11_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}

/* PS2 mouse */
void irq12_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}

/* FPU/co-CPU/inter-CPU */
void irq13_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}

/* primary ATA hard disk */
void irq14_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}

/* secondary ATA hard disk */
void irq15_handler(const _ATTR_MAYBE_UNUSED InterruptFrame* frame)
{
    pic8259_signal_eoi(1);
}
