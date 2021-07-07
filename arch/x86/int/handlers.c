/*
    Copyright (C) Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/x86/int/pic.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/x86/portio.h>

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
