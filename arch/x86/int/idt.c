/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/interrupt_frame.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/types.h>

/* genric TRAP exit function. */
asm(
    ".type isr_exit, @function                       \n"
    "isr_exit:                                       \n"
    "  addl $4, %esp # jump over the InterruptFrame* \n"
    "  popa                                          \n"
    "  addl $4, %esp # jump over the code            \n"
    "  iretl                                         \n"
);

/* TRAP entry that has the status code pushed on to the stack. */
#define TRAP_ENTRY_CODE(id)                                     \
_ATTR_NAKED void trap##id() {                                   \
    asm volatile(                                              \
        "pusha                                             \n" \
        "pushl %esp              # set the InterruptFrame* \n" \
        "call  trap"#id"_handler                            \n" \
        "jmp isr_exit                                      \n" \
    );                                                         \
}

/* TRAP entry that pushes a dummy status code on to the stack. */
#define TRAP_ENTRY_NO_CODE(id)                                 \
_ATTR_NAKED void trap##id() {                                  \
    asm volatile(                                             \
        "pushl $0                #push a fake code        \n" \
        "pusha                                            \n" \
        "pushl %esp              #set the InterruptFrame* \n" \
        "call  trap"#id"_handler                           \n" \
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
    asm volatile("lidt (%0)": : "b"(idtr));
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

void idt_init()
{
    interrupts_disable();
    
    for(size_t i = 0; i < 256; ++i)
        g_isrs[i] = dummy_cb;

#define COMMON_TRAP_GATE IDT_GATE_TYPE_TRAP_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT

    idt_encode_entry((ptr)trap0, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[0]);
    idt_encode_entry((ptr)trap1, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[1]);
    idt_encode_entry((ptr)trap2, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[2]);
    idt_encode_entry((ptr)trap3, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[3]);
    idt_encode_entry((ptr)trap4, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[4]);
    idt_encode_entry((ptr)trap5, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[5]);
    idt_encode_entry((ptr)trap6, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[6]);
    idt_encode_entry((ptr)trap7, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[7]);
    idt_encode_entry((ptr)trap8, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[8]);
    idt_encode_entry((ptr)trap9, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[9]);
    idt_encode_entry((ptr)trap10, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[10]);
    idt_encode_entry((ptr)trap11, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[11]);
    idt_encode_entry((ptr)trap12, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[12]);
    idt_encode_entry((ptr)trap13, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[13]);
    idt_encode_entry((ptr)trap14, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[14]);
    idt_encode_entry((ptr)trap15, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[15]);
    idt_encode_entry((ptr)trap16, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[16]);
    idt_encode_entry((ptr)trap17, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[17]);
    idt_encode_entry((ptr)trap18, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[18]);
    idt_encode_entry((ptr)trap19, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[19]);
    idt_encode_entry((ptr)trap20, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[20]);
    idt_encode_entry((ptr)trap21, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[21]);
    idt_encode_entry((ptr)trap22, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[22]);
    idt_encode_entry((ptr)trap23, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[23]);
    idt_encode_entry((ptr)trap24, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[24]);
    idt_encode_entry((ptr)trap25, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[25]);
    idt_encode_entry((ptr)trap26, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[26]);
    idt_encode_entry((ptr)trap27, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[27]);
    idt_encode_entry((ptr)trap28, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[28]);
    idt_encode_entry((ptr)trap29, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[29]);
    idt_encode_entry((ptr)trap30, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[30]);
    idt_encode_entry((ptr)trap31, SYSGDT_KERNEL_CS, COMMON_TRAP_GATE, &g_idt[31]);

#define COMMON_IRQ_GATE IDT_GATE_TYPE_INT_32 | IDT_DESC_PRIV_0 | IDT_INT_PRESENT
#define MASTER_PIC_OFFSET 32
#define SLAVE_PIC_OFFSET  (MASTER_PIC_OFFSET + 8)

    idt_encode_entry((ptr)irq0, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[0 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq1, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[1 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq2, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[2 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq3, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[3 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq4, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[4 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq5, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[5 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq6, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[6 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq7, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[7 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq8, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[8 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq9, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[9 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq10, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[10 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq11, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[11 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq12, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[12 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq13, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[13 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq14, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[14 + MASTER_PIC_OFFSET]);
    idt_encode_entry((ptr)irq15, SYSGDT_KERNEL_CS, COMMON_IRQ_GATE, &g_idt[15 + MASTER_PIC_OFFSET]);

    g_idtr.base = g_idt;
    g_idtr.limit = sizeof(g_idt) - 1;

    pic8259_remap(MASTER_PIC_OFFSET, SLAVE_PIC_OFFSET);
    idt_load(&g_idtr);

    interrupts_enable();
}

bool idt_register_isr(u8 irq, isr cb)
{
    g_isrs[irq] = cb;
    return true;
}

void trap0_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP0](frame, NULL);
}

void trap1_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP1](frame, NULL);
}

void trap2_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP2](frame, NULL);
}

void trap3_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP3](frame, NULL);
}

void trap4_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP4](frame, NULL);
}

void trap5_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP5](frame, NULL);
}

void trap6_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP6](frame, NULL);
}

void trap7_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP7](frame, NULL);
}

void trap8_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP8](frame, NULL);
}

void trap9_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP9](frame, NULL);
}

void trap10_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP10](frame, NULL);
}

void trap11_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP11](frame, NULL);
}

void trap12_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP12](frame, NULL);
}

void trap13_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP13](frame, NULL);
}

void trap14_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP14](frame, NULL);
}

void trap15_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP15](frame, NULL);
}

void trap16_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP16](frame, NULL);
}

void trap17_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP17](frame, NULL);
}

void trap18_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP18](frame, NULL);
}

void trap19_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP19](frame, NULL);
}

void trap20_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP20](frame, NULL);
}

void trap21_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP21](frame, NULL);
}

void trap22_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP22](frame, NULL);
}

void trap23_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP23](frame, NULL);
}

void trap24_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP24](frame, NULL);
}

void trap25_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP25](frame, NULL);
}

void trap26_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP26](frame, NULL);
}

void trap27_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP27](frame, NULL);
}

void trap28_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP28](frame, NULL);
}

void trap29_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP29](frame, NULL);
}

void trap30_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP30](frame, NULL);
}

void trap31_handler(const InterruptFrame* frame)
{
    g_isrs[TRAP31](frame, NULL);
}

/* PIT */
void irq0_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ0](frame, NULL);
    pic8259_signal_eoi(0);
}

/* keyboard */
void irq1_handler(const InterruptFrame* frame)
{
    unsigned char scan_code = port_inb(0x60); 
    g_isrs[IRQ1](frame, NULL);
    pic8259_signal_eoi(0);
}

/* used internally by the 2 PICs */
void irq2_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ2](frame, NULL);
    pic8259_signal_eoi(0);
}

/* COM2 */
void irq3_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ3](frame, NULL);
    pic8259_signal_eoi(0);
}

/* COM1 */
void irq4_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ4](frame, NULL);
    pic8259_signal_eoi(0);
}

/* LPT2 */
void irq5_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ5](frame, NULL);
    pic8259_signal_eoi(0);
}

/* floppy disk */
void irq6_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ6](frame, NULL);
    pic8259_signal_eoi(0);
}

/* LPT1 */
void irq7_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ7](frame, NULL);
    pic8259_signal_eoi(0);
}

/* CMOS RTC */
void irq8_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ8](frame, NULL);
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq9_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ9](frame, NULL);
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq10_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ10](frame, NULL);
    pic8259_signal_eoi(1);
}

/* free for peripherals */
void irq11_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ11](frame, NULL);
    pic8259_signal_eoi(1);
}

/* PS2 mouse */
void irq12_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ12](frame, NULL);
    pic8259_signal_eoi(1);
}

/* FPU/co-CPU/inter-CPU */
void irq13_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ13](frame, NULL);
    pic8259_signal_eoi(1);
}

/* primary ATA hard disk */
void irq14_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ14](frame, NULL);
    pic8259_signal_eoi(1);
}

/* secondary ATA hard disk */
void irq15_handler(const InterruptFrame* frame)
{
    g_isrs[IRQ15](frame, NULL);
    pic8259_signal_eoi(1);
}
