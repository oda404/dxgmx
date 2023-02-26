/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/interrupts.h>
#include <dxgmx/panic.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/exceptions.h>
#include <dxgmx/x86/gdt.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/interrupt_frame.h>
#include <dxgmx/x86/pic.h>
#include <dxgmx/x86/portio.h>

/* The interrupt service routine callbacks. */
static x86isr_t g_isrs[256];
/* The IDT */
static IDTEntry g_idt[256];
static IDTR g_idtr;
/* Offset of usable isrs. 32 on PIC8259 systems, as the first 32 isrs are for
 * exception handlers. Probably 48 on I/O APIC systems as the PIC isrs will be
 * unusable (due to spurious interrupts). */
static u8 g_isr_offset = 0;

static _ATTR_USED bool idt_is_irq_spurious()
{
    /* Is this correct ? */
    u8 isr1 = pic8259_get_isr(0);
    u8 isr2 = pic8259_get_isr(1);
    return isr1 == 0 && isr2 == 0;
}

/* Generic interrupt exit. */
__asm__(".type int_common_exit, @function                       \n"
        ".local int_common_exit                                 \n"
        "int_common_exit:                                       \n"
        "  addl $4, %esp # jump over the InterruptFrame* \n"
        "  popa                                          \n"
        "  addl $4, %esp # jump over the code            \n"
        "  iretl                                         \n");

/* TRAP entry that has a status code pushed on to the stack. */
#define INT_ENTRY_CODE(id)                                                            \
    static _ATTR_NAKED _ATTR_USED void int##id()                                      \
    {                                                                                 \
        __asm__ volatile(                                                             \
            "pusha                         # Push all registers*                  \n" \
            "pushl %esp                    # Set the InterruptFrame*              \n" \
            "movl $0, %ebp                 # Set ebp to NULL to stop stack traces \n" \
            "cld                           # Clear direction flag                 \n" \
            "call *(g_isrs + " #id " * 4)  # Call isr    \n"                          \
            "jmp int_common_exit                                \n");                 \
    }

/* TRAP entry that pushes a dummy status code on to the stack. */
#define INT_ENTRY(id)                                                                 \
    static _ATTR_NAKED _ATTR_USED void int##id()                                      \
    {                                                                                 \
        __asm__ volatile(                                                             \
            "pushl $0                      # Push fake code                       \n" \
            "pusha                         # Push all general registers           \n" \
            "pushl %esp                    # Set the InterruptFrame*              \n" \
            "movl $0, %ebp                 # Set ebp to NULL to stop stack traces \n" \
            "cld                           # Clear direction flag                 \n" \
            "call *(g_isrs + " #id " * 4)  # Call isr    \n"                          \
            "jmp int_common_exit                                \n");                 \
    }

/* Spurious interrupts won't even have their ISR called. I love the PIC :) */
#define INT_ENTRY_MAYBE_SPURIOUS(id)                                                  \
    static _ATTR_NAKED _ATTR_USED void int##id()                                      \
    {                                                                                 \
        __asm__ volatile(                                                             \
            "pushl $0                      # Push fake code                       \n" \
            "pusha                         # Push all general registers           \n" \
            "pushl %esp                    # Set the InterruptFrame*              \n" \
            "movl $0, %ebp                 # Set ebp to NULL to stop stack traces \n" \
            "cld                           # Clear direction flag                 \n" \
            "call idt_is_irq_spurious      # Check if it's spurious               \n" \
            "cmp $1, %eax                                                         \n" \
            "je int_common_exit            # Skip the isr call if it's spurious   \n" \
            "call *(g_isrs + " #id " * 4)  # Call isr    \n"                          \
            "jmp int_common_exit                                \n");                 \
    }

/* Exceptions */
INT_ENTRY(0)
INT_ENTRY(1)
INT_ENTRY(2)
INT_ENTRY(3)
INT_ENTRY(4)
INT_ENTRY(5)
INT_ENTRY(6)
INT_ENTRY(7)
INT_ENTRY_CODE(8)
INT_ENTRY(9)
INT_ENTRY_CODE(10)
INT_ENTRY_CODE(11)
INT_ENTRY_CODE(12)
INT_ENTRY_CODE(13)
INT_ENTRY_CODE(14)
INT_ENTRY(15)
INT_ENTRY(16)
INT_ENTRY_CODE(17)
INT_ENTRY(18)
INT_ENTRY(19)
INT_ENTRY(20)
INT_ENTRY(21)
INT_ENTRY(22)
INT_ENTRY(23)
INT_ENTRY(24)
INT_ENTRY(25)
INT_ENTRY(26)
INT_ENTRY(27)
INT_ENTRY(28)
INT_ENTRY(29)
INT_ENTRY_CODE(30)
INT_ENTRY(31)

/* PIC8259 mapped ISA interrupts */
INT_ENTRY(32)
INT_ENTRY(33)
INT_ENTRY(34)
INT_ENTRY(35)
INT_ENTRY(36)
INT_ENTRY(37)
INT_ENTRY(38)
INT_ENTRY_MAYBE_SPURIOUS(39)
INT_ENTRY(40)
INT_ENTRY(41)
INT_ENTRY(42)
INT_ENTRY(43)
INT_ENTRY(44)
INT_ENTRY(45)
INT_ENTRY(46)
INT_ENTRY_MAYBE_SPURIOUS(47)

/* Free for use. I/O APIC will probably map ISA interrupts somewhere here */
INT_ENTRY(48)
INT_ENTRY(49)
INT_ENTRY(50)
INT_ENTRY(51)
INT_ENTRY(52)
INT_ENTRY(53)
INT_ENTRY(54)
INT_ENTRY(55)
INT_ENTRY(56)
INT_ENTRY(57)
INT_ENTRY(58)
INT_ENTRY(59)
INT_ENTRY(60)
INT_ENTRY(61)
INT_ENTRY(62)
INT_ENTRY(63)
INT_ENTRY(64)
INT_ENTRY(65)
INT_ENTRY(66)
INT_ENTRY(67)
INT_ENTRY(68)
INT_ENTRY(69)
INT_ENTRY(70)
INT_ENTRY(71)
INT_ENTRY(72)
INT_ENTRY(73)
INT_ENTRY(74)
INT_ENTRY(75)
INT_ENTRY(76)
INT_ENTRY(77)
INT_ENTRY(78)
INT_ENTRY(79)
INT_ENTRY(80)
INT_ENTRY(81)
INT_ENTRY(82)
INT_ENTRY(83)
INT_ENTRY(84)
INT_ENTRY(85)
INT_ENTRY(86)
INT_ENTRY(87)
INT_ENTRY(88)
INT_ENTRY(89)
INT_ENTRY(90)
INT_ENTRY(91)
INT_ENTRY(92)
INT_ENTRY(93)
INT_ENTRY(94)
INT_ENTRY(95)
INT_ENTRY(96)
INT_ENTRY(97)
INT_ENTRY(98)
INT_ENTRY(99)
INT_ENTRY(100)
INT_ENTRY(101)
INT_ENTRY(102)
INT_ENTRY(103)
INT_ENTRY(104)
INT_ENTRY(105)
INT_ENTRY(106)
INT_ENTRY(107)
INT_ENTRY(108)
INT_ENTRY(109)
INT_ENTRY(110)
INT_ENTRY(111)
INT_ENTRY(112)
INT_ENTRY(113)
INT_ENTRY(114)
INT_ENTRY(115)
INT_ENTRY(116)
INT_ENTRY(117)
INT_ENTRY(118)
INT_ENTRY(119)
INT_ENTRY(120)
INT_ENTRY(121)
INT_ENTRY(122)
INT_ENTRY(123)
INT_ENTRY(124)
INT_ENTRY(125)
INT_ENTRY(126)
INT_ENTRY(127)
INT_ENTRY(128)
INT_ENTRY(129)
INT_ENTRY(130)
INT_ENTRY(131)
INT_ENTRY(132)
INT_ENTRY(133)
INT_ENTRY(134)
INT_ENTRY(135)
INT_ENTRY(136)
INT_ENTRY(137)
INT_ENTRY(138)
INT_ENTRY(139)
INT_ENTRY(140)
INT_ENTRY(141)
INT_ENTRY(142)
INT_ENTRY(143)
INT_ENTRY(144)
INT_ENTRY(145)
INT_ENTRY(146)
INT_ENTRY(147)
INT_ENTRY(148)
INT_ENTRY(149)
INT_ENTRY(150)
INT_ENTRY(151)
INT_ENTRY(152)
INT_ENTRY(153)
INT_ENTRY(154)
INT_ENTRY(155)
INT_ENTRY(156)
INT_ENTRY(157)
INT_ENTRY(158)
INT_ENTRY(159)
INT_ENTRY(160)
INT_ENTRY(161)
INT_ENTRY(162)
INT_ENTRY(163)
INT_ENTRY(164)
INT_ENTRY(165)
INT_ENTRY(166)
INT_ENTRY(167)
INT_ENTRY(168)
INT_ENTRY(169)
INT_ENTRY(170)
INT_ENTRY(171)
INT_ENTRY(172)
INT_ENTRY(173)
INT_ENTRY(174)
INT_ENTRY(175)
INT_ENTRY(176)
INT_ENTRY(177)
INT_ENTRY(178)
INT_ENTRY(179)
INT_ENTRY(180)
INT_ENTRY(181)
INT_ENTRY(182)
INT_ENTRY(183)
INT_ENTRY(184)
INT_ENTRY(185)
INT_ENTRY(186)
INT_ENTRY(187)
INT_ENTRY(188)
INT_ENTRY(189)
INT_ENTRY(190)
INT_ENTRY(191)
INT_ENTRY(192)
INT_ENTRY(193)
INT_ENTRY(194)
INT_ENTRY(195)
INT_ENTRY(196)
INT_ENTRY(197)
INT_ENTRY(198)
INT_ENTRY(199)
INT_ENTRY(200)
INT_ENTRY(201)
INT_ENTRY(202)
INT_ENTRY(203)
INT_ENTRY(204)
INT_ENTRY(205)
INT_ENTRY(206)
INT_ENTRY(207)
INT_ENTRY(208)
INT_ENTRY(209)
INT_ENTRY(210)
INT_ENTRY(211)
INT_ENTRY(212)
INT_ENTRY(213)
INT_ENTRY(214)
INT_ENTRY(215)
INT_ENTRY(216)
INT_ENTRY(217)
INT_ENTRY(218)
INT_ENTRY(219)
INT_ENTRY(220)
INT_ENTRY(221)
INT_ENTRY(222)
INT_ENTRY(223)
INT_ENTRY(224)
INT_ENTRY(225)
INT_ENTRY(226)
INT_ENTRY(227)
INT_ENTRY(228)
INT_ENTRY(229)
INT_ENTRY(230)
INT_ENTRY(231)
INT_ENTRY(232)
INT_ENTRY(233)
INT_ENTRY(234)
INT_ENTRY(235)
INT_ENTRY(236)
INT_ENTRY(237)
INT_ENTRY(238)
INT_ENTRY(239)
INT_ENTRY(240)
INT_ENTRY(241)
INT_ENTRY(242)
INT_ENTRY(243)
INT_ENTRY(244)
INT_ENTRY(245)
INT_ENTRY(246)
INT_ENTRY(247)
INT_ENTRY(248)
INT_ENTRY(249)
INT_ENTRY(250)
INT_ENTRY(251)
INT_ENTRY(252)
INT_ENTRY(253)
INT_ENTRY(254)
INT_ENTRY(255)

static void
idt_encode_entry(void* base, u16 selector, u8 flags, IDTEntry* entry)
{
    entry->base_0_15 = ((ptr)base & 0xFFFF);
    entry->base_16_31 = ((ptr)base & 0xFFFF0000) >> 16;
    entry->selector = selector;
    entry->unused = 0;
    entry->type = flags & 0b11111;
    entry->privilege = (flags & (0b11 << 5)) >> 5;
    entry->present = (bool)(flags & (1 << 7));
}

static void idt_encode_absent(void* base, IDTEntry* entry)
{
    entry->base_0_15 = ((ptr)base & 0xFFFF);
    entry->base_16_31 = ((ptr)base & 0xFFFF0000) >> 16;
    entry->unused = 0;
    entry->present = 0;
}

static void idt_encode_flags(u16 selector, u8 flags, IDTEntry* entry)
{
    entry->selector = selector;
    entry->type = flags & 0b11111;
    entry->privilege = (flags & (0b11 << 5)) >> 5;
    entry->present = (bool)(flags & (1 << 7));
}

static void idt_load(const IDTR* idtr)
{
    __asm__ volatile("lidt (%0)" : : "b"(idtr));
}

#define IDT_TASK32_GATE 0b10101
#define IDT_INT16_GATE 0b00110
#define IDT_TRAP16_GATE 0b00111
#define IDT_INT32_GATE 0b01110
#define IDT_TRAP32_GATE 0b01111

#define IDT_PRIVL0 (0 << 5)
#define IDT_PRIVL1 (1 << 5)
#define IDT_PRIVL2 (2 << 5)
#define IDT_PRIVL3 (3 << 5)

#define IDT_INT_PRESENT (1 << 7)

static void stub_trap(InterruptFrame*) {}

static void stub_irq(InterruptFrame*)
{
    interrupts_irq_done();
}

_INIT void idt_init()
{
    g_isr_offset = 32;

    for (size_t i = 0; i < 32; ++i)
        g_isrs[i] = stub_trap;

    for (size_t i = 32; i < 48; ++i)
        g_isrs[i] = stub_irq;

    /* Exceptions. The common ones get setup a bit later. */
    idt_encode_absent(int0, &g_idt[0]);
    idt_encode_absent(int1, &g_idt[1]);
    idt_encode_absent(int2, &g_idt[2]);
    idt_encode_absent(int3, &g_idt[3]);
    idt_encode_absent(int4, &g_idt[4]);
    idt_encode_absent(int5, &g_idt[5]);
    idt_encode_absent(int6, &g_idt[6]);
    idt_encode_absent(int7, &g_idt[7]);
    idt_encode_absent(int8, &g_idt[8]);
    idt_encode_absent(int9, &g_idt[9]);
    idt_encode_absent(int10, &g_idt[10]);
    idt_encode_absent(int11, &g_idt[11]);
    idt_encode_absent(int12, &g_idt[12]);
    idt_encode_absent(int13, &g_idt[13]);
    idt_encode_absent(int14, &g_idt[14]);
    idt_encode_absent(int15, &g_idt[15]);
    idt_encode_absent(int16, &g_idt[16]);
    idt_encode_absent(int17, &g_idt[17]);
    idt_encode_absent(int18, &g_idt[18]);
    idt_encode_absent(int19, &g_idt[19]);
    idt_encode_absent(int20, &g_idt[20]);
    idt_encode_absent(int21, &g_idt[21]);
    idt_encode_absent(int22, &g_idt[22]);
    idt_encode_absent(int23, &g_idt[23]);
    idt_encode_absent(int24, &g_idt[24]);
    idt_encode_absent(int25, &g_idt[25]);
    idt_encode_absent(int26, &g_idt[26]);
    idt_encode_absent(int27, &g_idt[27]);
    idt_encode_absent(int28, &g_idt[28]);
    idt_encode_absent(int29, &g_idt[29]);
    idt_encode_absent(int30, &g_idt[30]);
    idt_encode_absent(int31, &g_idt[31]);

#define ISA_IRQ IDT_INT32_GATE | IDT_PRIVL0 | IDT_INT_PRESENT

    /* ISA interrupts, these are predefined as they might fire anytime. */
    idt_encode_entry(int32, GDT_KERNEL_CS, ISA_IRQ, &g_idt[32]);
    idt_encode_entry(int33, GDT_KERNEL_CS, ISA_IRQ, &g_idt[33]);
    idt_encode_entry(int34, GDT_KERNEL_CS, ISA_IRQ, &g_idt[34]);
    idt_encode_entry(int35, GDT_KERNEL_CS, ISA_IRQ, &g_idt[35]);
    idt_encode_entry(int36, GDT_KERNEL_CS, ISA_IRQ, &g_idt[36]);
    idt_encode_entry(int37, GDT_KERNEL_CS, ISA_IRQ, &g_idt[37]);
    idt_encode_entry(int38, GDT_KERNEL_CS, ISA_IRQ, &g_idt[38]);
    idt_encode_entry(int39, GDT_KERNEL_CS, ISA_IRQ, &g_idt[39]);
    idt_encode_entry(int40, GDT_KERNEL_CS, ISA_IRQ, &g_idt[40]);
    idt_encode_entry(int41, GDT_KERNEL_CS, ISA_IRQ, &g_idt[41]);
    idt_encode_entry(int42, GDT_KERNEL_CS, ISA_IRQ, &g_idt[42]);
    idt_encode_entry(int43, GDT_KERNEL_CS, ISA_IRQ, &g_idt[43]);
    idt_encode_entry(int44, GDT_KERNEL_CS, ISA_IRQ, &g_idt[44]);
    idt_encode_entry(int45, GDT_KERNEL_CS, ISA_IRQ, &g_idt[45]);
    idt_encode_entry(int46, GDT_KERNEL_CS, ISA_IRQ, &g_idt[46]);
    idt_encode_entry(int47, GDT_KERNEL_CS, ISA_IRQ, &g_idt[47]);

#undef ISA_IRQ

    /* These should not fire unless they are setup by a driver, in which case
     * we'll also have an isr */
    idt_encode_absent(int48, &g_idt[48]);
    idt_encode_absent(int49, &g_idt[49]);
    idt_encode_absent(int50, &g_idt[50]);
    idt_encode_absent(int51, &g_idt[51]);
    idt_encode_absent(int52, &g_idt[52]);
    idt_encode_absent(int53, &g_idt[53]);
    idt_encode_absent(int54, &g_idt[54]);
    idt_encode_absent(int55, &g_idt[55]);
    idt_encode_absent(int56, &g_idt[56]);
    idt_encode_absent(int57, &g_idt[57]);
    idt_encode_absent(int58, &g_idt[58]);
    idt_encode_absent(int59, &g_idt[59]);
    idt_encode_absent(int60, &g_idt[60]);
    idt_encode_absent(int61, &g_idt[61]);
    idt_encode_absent(int62, &g_idt[62]);
    idt_encode_absent(int63, &g_idt[63]);
    idt_encode_absent(int64, &g_idt[64]);
    idt_encode_absent(int65, &g_idt[65]);
    idt_encode_absent(int66, &g_idt[66]);
    idt_encode_absent(int67, &g_idt[67]);
    idt_encode_absent(int68, &g_idt[68]);
    idt_encode_absent(int69, &g_idt[69]);
    idt_encode_absent(int70, &g_idt[70]);
    idt_encode_absent(int71, &g_idt[71]);
    idt_encode_absent(int72, &g_idt[72]);
    idt_encode_absent(int73, &g_idt[73]);
    idt_encode_absent(int74, &g_idt[74]);
    idt_encode_absent(int75, &g_idt[75]);
    idt_encode_absent(int76, &g_idt[76]);
    idt_encode_absent(int77, &g_idt[77]);
    idt_encode_absent(int78, &g_idt[78]);
    idt_encode_absent(int79, &g_idt[79]);
    idt_encode_absent(int80, &g_idt[80]);
    idt_encode_absent(int81, &g_idt[81]);
    idt_encode_absent(int82, &g_idt[82]);
    idt_encode_absent(int83, &g_idt[83]);
    idt_encode_absent(int84, &g_idt[84]);
    idt_encode_absent(int85, &g_idt[85]);
    idt_encode_absent(int86, &g_idt[86]);
    idt_encode_absent(int87, &g_idt[87]);
    idt_encode_absent(int88, &g_idt[88]);
    idt_encode_absent(int89, &g_idt[89]);
    idt_encode_absent(int90, &g_idt[90]);
    idt_encode_absent(int91, &g_idt[91]);
    idt_encode_absent(int92, &g_idt[92]);
    idt_encode_absent(int93, &g_idt[93]);
    idt_encode_absent(int94, &g_idt[94]);
    idt_encode_absent(int95, &g_idt[95]);
    idt_encode_absent(int96, &g_idt[96]);
    idt_encode_absent(int97, &g_idt[97]);
    idt_encode_absent(int98, &g_idt[98]);
    idt_encode_absent(int99, &g_idt[99]);
    idt_encode_absent(int100, &g_idt[100]);
    idt_encode_absent(int101, &g_idt[101]);
    idt_encode_absent(int102, &g_idt[102]);
    idt_encode_absent(int103, &g_idt[103]);
    idt_encode_absent(int104, &g_idt[104]);
    idt_encode_absent(int105, &g_idt[105]);
    idt_encode_absent(int106, &g_idt[106]);
    idt_encode_absent(int107, &g_idt[107]);
    idt_encode_absent(int108, &g_idt[108]);
    idt_encode_absent(int109, &g_idt[109]);
    idt_encode_absent(int110, &g_idt[110]);
    idt_encode_absent(int111, &g_idt[111]);
    idt_encode_absent(int112, &g_idt[112]);
    idt_encode_absent(int113, &g_idt[113]);
    idt_encode_absent(int114, &g_idt[114]);
    idt_encode_absent(int115, &g_idt[115]);
    idt_encode_absent(int116, &g_idt[116]);
    idt_encode_absent(int117, &g_idt[117]);
    idt_encode_absent(int118, &g_idt[118]);
    idt_encode_absent(int119, &g_idt[119]);
    idt_encode_absent(int120, &g_idt[120]);
    idt_encode_absent(int121, &g_idt[121]);
    idt_encode_absent(int122, &g_idt[122]);
    idt_encode_absent(int123, &g_idt[123]);
    idt_encode_absent(int124, &g_idt[124]);
    idt_encode_absent(int125, &g_idt[125]);
    idt_encode_absent(int126, &g_idt[126]);
    idt_encode_absent(int127, &g_idt[127]);
    idt_encode_absent(int128, &g_idt[128]);
    idt_encode_absent(int129, &g_idt[129]);
    idt_encode_absent(int130, &g_idt[130]);
    idt_encode_absent(int131, &g_idt[131]);
    idt_encode_absent(int132, &g_idt[132]);
    idt_encode_absent(int133, &g_idt[133]);
    idt_encode_absent(int134, &g_idt[134]);
    idt_encode_absent(int135, &g_idt[135]);
    idt_encode_absent(int136, &g_idt[136]);
    idt_encode_absent(int137, &g_idt[137]);
    idt_encode_absent(int138, &g_idt[138]);
    idt_encode_absent(int139, &g_idt[139]);
    idt_encode_absent(int140, &g_idt[140]);
    idt_encode_absent(int141, &g_idt[141]);
    idt_encode_absent(int142, &g_idt[142]);
    idt_encode_absent(int143, &g_idt[143]);
    idt_encode_absent(int144, &g_idt[144]);
    idt_encode_absent(int145, &g_idt[145]);
    idt_encode_absent(int146, &g_idt[146]);
    idt_encode_absent(int147, &g_idt[147]);
    idt_encode_absent(int148, &g_idt[148]);
    idt_encode_absent(int149, &g_idt[149]);
    idt_encode_absent(int150, &g_idt[150]);
    idt_encode_absent(int151, &g_idt[151]);
    idt_encode_absent(int152, &g_idt[152]);
    idt_encode_absent(int153, &g_idt[153]);
    idt_encode_absent(int154, &g_idt[154]);
    idt_encode_absent(int155, &g_idt[155]);
    idt_encode_absent(int156, &g_idt[156]);
    idt_encode_absent(int157, &g_idt[157]);
    idt_encode_absent(int158, &g_idt[158]);
    idt_encode_absent(int159, &g_idt[159]);
    idt_encode_absent(int160, &g_idt[160]);
    idt_encode_absent(int161, &g_idt[161]);
    idt_encode_absent(int162, &g_idt[162]);
    idt_encode_absent(int163, &g_idt[163]);
    idt_encode_absent(int164, &g_idt[164]);
    idt_encode_absent(int165, &g_idt[165]);
    idt_encode_absent(int166, &g_idt[166]);
    idt_encode_absent(int167, &g_idt[167]);
    idt_encode_absent(int168, &g_idt[168]);
    idt_encode_absent(int169, &g_idt[169]);
    idt_encode_absent(int170, &g_idt[170]);
    idt_encode_absent(int171, &g_idt[171]);
    idt_encode_absent(int172, &g_idt[172]);
    idt_encode_absent(int173, &g_idt[173]);
    idt_encode_absent(int174, &g_idt[174]);
    idt_encode_absent(int175, &g_idt[175]);
    idt_encode_absent(int176, &g_idt[176]);
    idt_encode_absent(int177, &g_idt[177]);
    idt_encode_absent(int178, &g_idt[178]);
    idt_encode_absent(int179, &g_idt[179]);
    idt_encode_absent(int180, &g_idt[180]);
    idt_encode_absent(int181, &g_idt[181]);
    idt_encode_absent(int182, &g_idt[182]);
    idt_encode_absent(int183, &g_idt[183]);
    idt_encode_absent(int184, &g_idt[184]);
    idt_encode_absent(int185, &g_idt[185]);
    idt_encode_absent(int186, &g_idt[186]);
    idt_encode_absent(int187, &g_idt[187]);
    idt_encode_absent(int188, &g_idt[188]);
    idt_encode_absent(int189, &g_idt[189]);
    idt_encode_absent(int190, &g_idt[190]);
    idt_encode_absent(int191, &g_idt[191]);
    idt_encode_absent(int192, &g_idt[192]);
    idt_encode_absent(int193, &g_idt[193]);
    idt_encode_absent(int194, &g_idt[194]);
    idt_encode_absent(int195, &g_idt[195]);
    idt_encode_absent(int196, &g_idt[196]);
    idt_encode_absent(int197, &g_idt[197]);
    idt_encode_absent(int198, &g_idt[198]);
    idt_encode_absent(int199, &g_idt[199]);
    idt_encode_absent(int200, &g_idt[200]);
    idt_encode_absent(int201, &g_idt[201]);
    idt_encode_absent(int202, &g_idt[202]);
    idt_encode_absent(int203, &g_idt[203]);
    idt_encode_absent(int204, &g_idt[204]);
    idt_encode_absent(int205, &g_idt[205]);
    idt_encode_absent(int206, &g_idt[206]);
    idt_encode_absent(int207, &g_idt[207]);
    idt_encode_absent(int208, &g_idt[208]);
    idt_encode_absent(int209, &g_idt[209]);
    idt_encode_absent(int210, &g_idt[210]);
    idt_encode_absent(int211, &g_idt[211]);
    idt_encode_absent(int212, &g_idt[212]);
    idt_encode_absent(int213, &g_idt[213]);
    idt_encode_absent(int214, &g_idt[214]);
    idt_encode_absent(int215, &g_idt[215]);
    idt_encode_absent(int216, &g_idt[216]);
    idt_encode_absent(int217, &g_idt[217]);
    idt_encode_absent(int218, &g_idt[218]);
    idt_encode_absent(int219, &g_idt[219]);
    idt_encode_absent(int220, &g_idt[220]);
    idt_encode_absent(int221, &g_idt[221]);
    idt_encode_absent(int222, &g_idt[222]);
    idt_encode_absent(int223, &g_idt[223]);
    idt_encode_absent(int224, &g_idt[224]);
    idt_encode_absent(int225, &g_idt[225]);
    idt_encode_absent(int226, &g_idt[226]);
    idt_encode_absent(int227, &g_idt[227]);
    idt_encode_absent(int228, &g_idt[228]);
    idt_encode_absent(int229, &g_idt[229]);
    idt_encode_absent(int230, &g_idt[230]);
    idt_encode_absent(int231, &g_idt[231]);
    idt_encode_absent(int232, &g_idt[232]);
    idt_encode_absent(int233, &g_idt[233]);
    idt_encode_absent(int234, &g_idt[234]);
    idt_encode_absent(int235, &g_idt[235]);
    idt_encode_absent(int236, &g_idt[236]);
    idt_encode_absent(int237, &g_idt[237]);
    idt_encode_absent(int238, &g_idt[238]);
    idt_encode_absent(int239, &g_idt[239]);
    idt_encode_absent(int240, &g_idt[240]);
    idt_encode_absent(int241, &g_idt[241]);
    idt_encode_absent(int242, &g_idt[242]);
    idt_encode_absent(int243, &g_idt[243]);
    idt_encode_absent(int244, &g_idt[244]);
    idt_encode_absent(int245, &g_idt[245]);
    idt_encode_absent(int246, &g_idt[246]);
    idt_encode_absent(int247, &g_idt[247]);
    idt_encode_absent(int248, &g_idt[248]);
    idt_encode_absent(int249, &g_idt[249]);
    idt_encode_absent(int250, &g_idt[250]);
    idt_encode_absent(int251, &g_idt[251]);
    idt_encode_absent(int252, &g_idt[252]);
    idt_encode_absent(int253, &g_idt[253]);
    idt_encode_absent(int254, &g_idt[254]);
    idt_encode_absent(int255, &g_idt[255]);

    g_idtr.base = g_idt;
    g_idtr.limit = sizeof(g_idt) - 1;

    pic8259_remap(g_isr_offset, g_isr_offset + 8);
    idt_load(&g_idtr);

    exceptions_set_up_common_handlers();

    /* My Lenovo laptop has the PIC masked on boot. */
    pic8259_set_mask(0, 0);
    pic8259_set_mask(0, 1);
    interrupts_enable_irqs();
}

int idt_register_trap_isr(intn_t n, u8 ring, x86isr_t cb)
{
    u8 flags = IDT_TRAP32_GATE | IDT_INT_PRESENT;
    if (ring == 0)
        flags |= IDT_PRIVL0;
    else if (ring == 3)
        flags |= IDT_PRIVL3;
    else
        return -1;

    g_isrs[n] = cb;
    idt_encode_flags(GDT_KERNEL_CS, flags, &g_idt[n]);
    return 0;
}

int idt_register_irq_isr(intn_t n, x86isr_t cb)
{
    u8 flags = IDT_INT32_GATE | IDT_INT_PRESENT | IDT_PRIVL0;

    g_isrs[n] = cb;
    idt_encode_flags(GDT_KERNEL_CS, flags, &g_idt[n]);
    return 0;
}
