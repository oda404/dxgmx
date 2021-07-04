/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/x86/pic.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/x86/portio.h>

/* PIT */
void irq0_handler()
{
    pic8259_signal_eoi(0);
}

/* keyboard */
void irq1_handler()
{
    unsigned char scan_code = port_inb(0x60); 
    kprintf("%d\n", scan_code);
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
