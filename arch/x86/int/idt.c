/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/types.h>
#include<dxgmx/attrs.h>
#include<dxgmx/panic.h>

/* genric TRAP exit function. */
__asm__(
    ".type isr_exit, @function                       \n"
    "isr_exit:                                       \n"
    "  addl $4, %esp # jump over the InterruptFrame* \n"
    "  popa                                          \n"
    "  addl $4, %esp # jump over the code            \n"
    "  iretl                                         \n"
);

/* TRAP entry that has the status code pushed on to the stack. */
#define TRAP_ENTRY_CODE(id)                                     \
_ATTR_NAKED static void trap##id() {                                   \
    __asm__ volatile(                                              \
        ".local trap"#id"_handler                          \n" \
        "pusha                                             \n" \
        "pushl %esp              # set the InterruptFrame* \n" \
        "call  trap"#id"_handler                            \n" \
        "jmp isr_exit                                      \n" \
    );                                                         \
}

/* TRAP entry that pushes a dummy status code on to the stack. */
#define TRAP_ENTRY_NO_CODE(id)                                 \
_ATTR_NAKED static void trap##id() {                                  \
    __asm__ volatile(                                             \
        ".local trap"#id"_handler                          \n" \
        "pushl $0                #push a fake code        \n" \
        "pusha                                            \n" \
        "pushl %esp              #set the InterruptFrame* \n" \
        "call  trap"#id"_handler                           \n" \
        "jmp isr_exit                                     \n" \
    );                                                        \
}

#define IRQ_ENTRY(id)                 \
_ATTR_NAKED static void irq##id() {          \
    __asm__ volatile(                     \
        ".local irq"#id"_handler  \n" \
        "pushl $0                 \n" \
        "pusha                    \n" \
        "pushl %esp               \n" \
        "call irq"#id"_handler    \n" \
        "jmp isr_exit             \n" \
    );                                \
}

TRAP_ENTRY_NO_CODE(0)
TRAP_ENTRY_NO_CODE(1)
TRAP_ENTRY_NO_CODE(2)
TRAP_ENTRY_NO_CODE(3)
TRAP_ENTRY_NO_CODE(4)
TRAP_ENTRY_NO_CODE(5)
TRAP_ENTRY_NO_CODE(6)
TRAP_ENTRY_NO_CODE(7)
TRAP_ENTRY_CODE(8)
TRAP_ENTRY_NO_CODE(9)
TRAP_ENTRY_CODE(10)
TRAP_ENTRY_CODE(11)
TRAP_ENTRY_CODE(12)
TRAP_ENTRY_CODE(13)
TRAP_ENTRY_CODE(14)
TRAP_ENTRY_NO_CODE(15)
TRAP_ENTRY_NO_CODE(16)
TRAP_ENTRY_CODE(17)
TRAP_ENTRY_NO_CODE(18)
TRAP_ENTRY_NO_CODE(19)
TRAP_ENTRY_NO_CODE(20)
TRAP_ENTRY_NO_CODE(21)
TRAP_ENTRY_NO_CODE(22)
TRAP_ENTRY_NO_CODE(23)
TRAP_ENTRY_NO_CODE(24)
TRAP_ENTRY_NO_CODE(25)
TRAP_ENTRY_NO_CODE(26)
TRAP_ENTRY_NO_CODE(27)
TRAP_ENTRY_NO_CODE(28)
TRAP_ENTRY_NO_CODE(29)
TRAP_ENTRY_CODE(30)
TRAP_ENTRY_NO_CODE(31)

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

static void idt_encode_entry(
    ptr base,
    uint16_t selector,
    uint8_t  flags,
    IDTEntry *entry
)
{
#ifdef _X86_
    entry->base_0_15  = (base & 0xFFFF);
    entry->base_16_31 = (base & 0xFFFF0000) >> 16;
    entry->selector   = selector;
    entry->unused     = 0;
    entry->type       = flags & 0b11111;
    entry->privilege  = flags & (0b11 << 5);
    entry->present    = (bool)(flags & (1 << 7));
#endif // _X86_
}

static void idt_load(const IDTR *idtr)
{
    __asm__ volatile("lidt (%0)": : "b"(idtr));
}

static u32 g_isr_trashcan = 0;

static void dummy_cb(
    const InterruptFrame _ATTR_MAYBE_UNUSED *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
    ++g_isr_trashcan;
}

u32 idt_get_isr_trashcan()
{
    return g_isr_trashcan;
}

/* TRAP callbacks */
static isr g_isrs[256];
static IDTEntry g_idt[256];
static IDTR g_idtr;

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

_INIT void idt_init()
{
    interrupts_disable();
    
    for(size_t i = 0; i < 256; ++i)
        g_isrs[i] = dummy_cb;

#define COMMON_TRAP_GATE IDT_GATE_TYPE_TRAP_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT

    idt_encode_entry((ptr)trap0, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[0]);
    idt_encode_entry((ptr)trap1, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[1]);
    idt_encode_entry((ptr)trap2, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[2]);
    idt_encode_entry((ptr)trap3, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[3]);
    idt_encode_entry((ptr)trap4, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[4]);
    idt_encode_entry((ptr)trap5, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[5]);
    idt_encode_entry((ptr)trap6, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[6]);
    idt_encode_entry((ptr)trap7, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[7]);
    idt_encode_entry((ptr)trap8, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[8]);
    idt_encode_entry((ptr)trap9, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[9]);
    idt_encode_entry((ptr)trap10, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[10]);
    idt_encode_entry((ptr)trap11, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[11]);
    idt_encode_entry((ptr)trap12, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[12]);
    idt_encode_entry((ptr)trap13, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[13]);
    idt_encode_entry((ptr)trap14, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[14]);
    idt_encode_entry((ptr)trap15, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[15]);
    idt_encode_entry((ptr)trap16, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[16]);
    idt_encode_entry((ptr)trap17, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[17]);
    idt_encode_entry((ptr)trap18, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[18]);
    idt_encode_entry((ptr)trap19, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[19]);
    idt_encode_entry((ptr)trap20, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[20]);
    idt_encode_entry((ptr)trap21, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[21]);
    idt_encode_entry((ptr)trap22, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[22]);
    idt_encode_entry((ptr)trap23, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[23]);
    idt_encode_entry((ptr)trap24, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[24]);
    idt_encode_entry((ptr)trap25, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[25]);
    idt_encode_entry((ptr)trap26, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[26]);
    idt_encode_entry((ptr)trap27, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[27]);
    idt_encode_entry((ptr)trap28, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[28]);
    idt_encode_entry((ptr)trap29, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[29]);
    idt_encode_entry((ptr)trap30, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[30]);
    idt_encode_entry((ptr)trap31, GDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[31]);

#define COMMON_IRQ_GATE IDT_GATE_TYPE_INT_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT
#define MASTER_PIC_OFFSET 32
#define SLAVE_PIC_OFFSET  (MASTER_PIC_OFFSET + 8)

    idt_encode_entry((ptr)irq0, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[0 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq1, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[1 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq2, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[2 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq3, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[3 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq4, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[4 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq5, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[5 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq6, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[6 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq7, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[7 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq8, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[8 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq9, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[9 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq10, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[10 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq11, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[11 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq12, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[12 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq13, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[13 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq14, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[14 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq15, GDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[15 + MASTER_PIC_OFFSET]);

    g_idtr.base = g_idt;
    g_idtr.limit = sizeof(g_idt) - 1;

    pic8259_remap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
    idt_load(&g_idtr);

    interrupts_enable();
}

_INIT bool idt_register_isr(u8 irq, isr cb)
{
    g_isrs[irq] = cb;
    return true;
}

#define TRAP_HANDLER_NO_DATA(n) \
_ATTR_MAYBE_UNUSED static void \
trap##n##_handler(const InterruptFrame *frame) { \
    panic("trap %d\n", n); \
    g_isrs[TRAP##n](frame, NULL); \
}

TRAP_HANDLER_NO_DATA(0);
TRAP_HANDLER_NO_DATA(1);
TRAP_HANDLER_NO_DATA(2);
TRAP_HANDLER_NO_DATA(3);
TRAP_HANDLER_NO_DATA(4);
TRAP_HANDLER_NO_DATA(5);
TRAP_HANDLER_NO_DATA(6);
TRAP_HANDLER_NO_DATA(7);
TRAP_HANDLER_NO_DATA(8);
TRAP_HANDLER_NO_DATA(9);
TRAP_HANDLER_NO_DATA(10);
TRAP_HANDLER_NO_DATA(11);
TRAP_HANDLER_NO_DATA(12);
TRAP_HANDLER_NO_DATA(13);
TRAP_HANDLER_NO_DATA(14);
TRAP_HANDLER_NO_DATA(15);
TRAP_HANDLER_NO_DATA(16);
TRAP_HANDLER_NO_DATA(17);
TRAP_HANDLER_NO_DATA(18);
TRAP_HANDLER_NO_DATA(19);
TRAP_HANDLER_NO_DATA(20);
TRAP_HANDLER_NO_DATA(21);
TRAP_HANDLER_NO_DATA(22);
TRAP_HANDLER_NO_DATA(23);
TRAP_HANDLER_NO_DATA(24);
TRAP_HANDLER_NO_DATA(25);
TRAP_HANDLER_NO_DATA(26);
TRAP_HANDLER_NO_DATA(27);
TRAP_HANDLER_NO_DATA(28);
TRAP_HANDLER_NO_DATA(29);
TRAP_HANDLER_NO_DATA(30);
TRAP_HANDLER_NO_DATA(31);

#define IRQ_HANDLER_NO_DATA(n) \
_ATTR_MAYBE_UNUSED static void \
irq##n##_handler(const InterruptFrame *frame) { \
    g_isrs[IRQ##n](frame, NULL); \
    pic8259_signal_eoi(n < 8 ? 0 : 1); \
}

/* PIT */
IRQ_HANDLER_NO_DATA(0)

/* PS2 keyboard */
_ATTR_MAYBE_UNUSED static void 
irq1_handler(const InterruptFrame* frame)
{
    unsigned char scan_code = port_inb(0x60); 
    g_isrs[IRQ1](frame, NULL);
    pic8259_signal_eoi(0);
}

/* used internally by the 2 PICs */
IRQ_HANDLER_NO_DATA(2)

/* COM2 */
IRQ_HANDLER_NO_DATA(3)

/* COM1 */
IRQ_HANDLER_NO_DATA(4)

/* LPT2 */
IRQ_HANDLER_NO_DATA(5)

/* floppy disk */
IRQ_HANDLER_NO_DATA(6)

/* LPT1 */
IRQ_HANDLER_NO_DATA(7)

/* CMOS RTC */
IRQ_HANDLER_NO_DATA(8)

/* free for peripherals */
IRQ_HANDLER_NO_DATA(9)

/* free for peripherals */
IRQ_HANDLER_NO_DATA(10)

/* free for peripherals */
IRQ_HANDLER_NO_DATA(11)

/* PS2 mouse */
IRQ_HANDLER_NO_DATA(12)

/* FPU/co-CPU/inter-CPU */
IRQ_HANDLER_NO_DATA(13)

/* primary ATA hard disk */
IRQ_HANDLER_NO_DATA(14)

/* secondary ATA hard disk */
IRQ_HANDLER_NO_DATA(15)
