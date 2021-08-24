/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/sysidt.h>
#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/types.h>
#include<dxgmx/attrs.h>

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
    interrupt_callback cb;
} InterruptCallbacks;

#define INTS_CALLBACKS_MAX 48
static InterruptCallbacks g_int_cbs[INTS_CALLBACKS_MAX];

static void dummy_cb(
    const InterruptFrame _ATTR_MAYBE_UNUSED *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
    static u32 trashcan = 0;
    ++trashcan;
}

void sysidt_init()
{
    interrupts_disable();
    
    for(size_t i = 0; i < INTS_CALLBACKS_MAX; ++i)
        g_int_cbs[i].cb = dummy_cb;

#define COMMON_TRAP_GATE IDT_GATE_TYPE_TRAP_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT

    idt_encode_entry((ptr)isr0, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[0]);
    idt_encode_entry((ptr)isr1, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[1]);
    idt_encode_entry((ptr)isr2, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[2]);
    idt_encode_entry((ptr)isr3, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[3]);
    idt_encode_entry((ptr)isr4, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[4]);
    idt_encode_entry((ptr)isr5, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[5]);
    idt_encode_entry((ptr)isr6, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[6]);
    idt_encode_entry((ptr)isr7, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[7]);
    idt_encode_entry((ptr)isr8, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[8]);
    idt_encode_entry((ptr)isr9, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[9]);
    idt_encode_entry((ptr)isr10, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[10]);
    idt_encode_entry((ptr)isr11, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[11]);
    idt_encode_entry((ptr)isr12, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[12]);
    idt_encode_entry((ptr)isr13, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[13]);
    idt_encode_entry((ptr)isr14, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[14]);
    idt_encode_entry((ptr)isr15, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[15]);
    idt_encode_entry((ptr)isr16, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[16]);
    idt_encode_entry((ptr)isr17, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[17]);
    idt_encode_entry((ptr)isr18, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[18]);
    idt_encode_entry((ptr)isr19, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[19]);
    idt_encode_entry((ptr)isr20, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[20]);
    idt_encode_entry((ptr)isr21, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[21]);
    idt_encode_entry((ptr)isr22, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[22]);
    idt_encode_entry((ptr)isr23, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[23]);
    idt_encode_entry((ptr)isr24, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[24]);
    idt_encode_entry((ptr)isr25, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[25]);
    idt_encode_entry((ptr)isr26, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[26]);
    idt_encode_entry((ptr)isr27, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[27]);
    idt_encode_entry((ptr)isr28, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[28]);
    idt_encode_entry((ptr)isr29, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[29]);
    idt_encode_entry((ptr)isr30, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[30]);
    idt_encode_entry((ptr)isr31, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &idt[31]);

#define COMMON_IRQ_GATE IDT_GATE_TYPE_INT_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT

    idt_encode_entry((ptr)irq0, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[0 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq1, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[1 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq2, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[2 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq3, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[3 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq4, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[4 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq5, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[5 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq6, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[6 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq7, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[7 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq8, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[8 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq9, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[9 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq10, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[10 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq11, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[11 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq12, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[12 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq13, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[13 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq14, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[14 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq15, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &idt[15 + MASTER_PIC_OFFSET]);

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

    g_int_cbs[interrupt_n].cb = callback;

    return 0;
}

void isr0_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR0].cb(frame, NULL);
}

void isr1_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR1].cb(frame, NULL);
}

void isr2_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR2].cb(frame, NULL);
}

void isr3_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR3].cb(frame, NULL);
}

void isr4_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR4].cb(frame, NULL);
}

void isr5_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR5].cb(frame, NULL);
}

void isr6_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR6].cb(frame, NULL);
}

void isr7_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR7].cb(frame, NULL);
}

void isr8_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR8].cb(frame, NULL);
}

void isr9_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR9].cb(frame, NULL);
}

void isr10_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR10].cb(frame, NULL);
}

void isr11_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR11].cb(frame, NULL);
}

void isr12_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR12].cb(frame, NULL);
}

void isr13_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR13].cb(frame, NULL);
}

void isr14_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR14].cb(frame, NULL);
}

void isr15_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR15].cb(frame, NULL);
}

void isr16_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR16].cb(frame, NULL);
}

void isr17_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR17].cb(frame, NULL);
}

void isr18_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR18].cb(frame, NULL);
}

void isr19_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR19].cb(frame, NULL);
}

void isr20_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR20].cb(frame, NULL);
}

void isr21_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR21].cb(frame, NULL);
}

void isr22_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR22].cb(frame, NULL);
}

void isr23_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR23].cb(frame, NULL);
}

void isr24_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR24].cb(frame, NULL);
}

void isr25_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR25].cb(frame, NULL);
}

void isr26_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR26].cb(frame, NULL);
}

void isr27_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR27].cb(frame, NULL);
}

void isr28_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR28].cb(frame, NULL);
}

void isr29_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR29].cb(frame, NULL);
}

void isr30_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR30].cb(frame, NULL);
}

void isr31_handler(const InterruptFrame* frame)
{
    g_int_cbs[ISR31].cb(frame, NULL);
}

/* PIT */
void irq0_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ0].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* keyboard */
void irq1_handler(const InterruptFrame* frame)
{
    unsigned char scan_code = port_inb(0x60); 
    g_int_cbs[IRQ1].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* used internally by the 2 PICs */
void irq2_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ2].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* COM2 */
void irq3_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ3].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* COM1 */
void irq4_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ4].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* LPT2 */
void irq5_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ5].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* floppy disk */
void irq6_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ6].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* LPT1 */
void irq7_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ7].cb(frame, NULL);
    pic8259_signal_eoi(0);
}

/* CMOS RTC */
void irq8_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ8].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq9_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ9].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq10_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ10].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq11_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ11].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* PS2 mouse */
void irq12_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ12].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* FPU/co-CPU/inter-CPU */
void irq13_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ13].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* primary ATA hard disk */
void irq14_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ14].cb(frame, NULL);
    pic8259_signal_eoi(1);
}

/* secondary ATA hard disk */
void irq15_handler(const InterruptFrame* frame)
{
    g_int_cbs[IRQ15].cb(frame, NULL);
    pic8259_signal_eoi(1);
}
