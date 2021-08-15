/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/pic.h>
#include<dxgmx/x86/portio.h>

#define PIC_MASTER_PORT_COMMAND 0x20
#define PIC_MASTER_PORT_DATA    0x21
#define PIC_SLAVE_PORT_COMMAND  0xA0
#define PIC_SLAVE_PORT_DATA     0xA1

#define PIC_OWC3_READ_IRR       0xA
#define PIC_OWC3_READ_ISR       0xB

/* 
 * ICW - Initialization Command Word
 * There are a total of 4 ICWs.
 * If there is only one PIC, only ICW1, ICW2, ICW4
 * need to be programmed.
 * If the system is in cascade mode (2 or more PICs) ICW3
 * must also be programmed.
*/

/* Init sequence */
/* ICW1 must be programmed on the command port */
/* ICW2, ICW3(?), ICW4 follow on the data port */

/* ICW1 Programs the basic operation (1 byte, bit 4 must be set for init sequence) */

/* ICW4 is needed */
#define PIC_ICW1_ICW4_NEEDED  (1 << 0)
/* single mode, if not set, cascade mode */
#define PIC_ICW1_SINGLE       (1 << 1)
/* Only used for 8 bit 8085 CPUs | 
 * call address interval of 4, if not set, call address interval is 8
*/
#define PIC_ICW1_INTERVAL4    (1 << 2)
/* level triggered mode, if not set, edge triggered mode */
#define PIC_ICW1_LEVEL_TRIGG  (1 << 3)
#define PIC_ICW1_INIT         (1 << 4)
/* idk what the fuck bits 5-7 do exactly, but they are for 8 bit 8085 CPUs */

/* ICW2 offset for interrupt vector (1 byte) */

/* ICW3 (1 byte)
 * For the master, indicates what pins are used 
 * to connect the slave(s).
 * For the slave(s), indicates what pin is 
 * used to connect to it's master.
*/

/* ICW4 (1 byte) */

/* 8086 mode, if not set, 8085 8 bit mode */
#define PIC_ICW4_8086         (1 << 0)
/* interrupt automatically resets the
 * interrupt request pin and doesn't modify priority,
*/
#define PIC_ICW4_AUTO_EOI     (1 << 1)
/* buffered mode/master */
#define PIC_ICW4_BUFF_MASTER  (3 << 2)
/* buffered mode/slave */
#define PIC_ICW4_BUFF_SLAVE   (2 << 2)
/* 
 * special fully nested mode. 
 * Allows the master to recognize the highest priority
 * interrupt request from a slave while it's processing another
 * interrupt from a slave.
*/
#define PIC_ICW4_SFNM         (1 << 4)

void pic8259_remap(uint8_t master_offset, uint8_t slave_offset)
{
    asm volatile("cli");

    uint8_t master_mask;
    uint8_t slave_mask;

    /* save irq masks to restore after reinit */
    master_mask = port_inb(PIC_MASTER_PORT_DATA);
    slave_mask = port_inb(PIC_SLAVE_PORT_DATA);

    /* ICW1 for the master */
    port_outb(
        PIC_ICW1_INIT | PIC_ICW1_ICW4_NEEDED, 
        PIC_MASTER_PORT_COMMAND
    );
    /* ICW1 for the slave */
    port_outb(
        PIC_ICW1_INIT | PIC_ICW1_ICW4_NEEDED, 
        PIC_SLAVE_PORT_COMMAND
    );

    /* ICW2 for the master */
    port_outb(master_offset, PIC_MASTER_PORT_DATA);
    /* ICW2 for the slave */
    port_outb(slave_offset, PIC_SLAVE_PORT_DATA);

    /* ICW3 | the master has a slave PIC at it's IRQ2 */
    port_outb(0b100, PIC_MASTER_PORT_DATA);
    /* ICW3 | the slave is connected to the master at it's IRQ1 */
    port_outb(0b10, PIC_SLAVE_PORT_DATA);

    /* ICW4 for the master */
    port_outb(PIC_ICW4_8086, PIC_MASTER_PORT_DATA);
    /* ICW4 for the slave */
    port_outb(PIC_ICW4_8086, PIC_SLAVE_PORT_DATA);

    /* init sequence is completed, set the irq masks again */
    port_outb(master_mask, PIC_MASTER_PORT_DATA);
    port_outb(slave_mask, PIC_SLAVE_PORT_DATA);

    asm volatile("sti");
}

void pic8259_mask_irq_line(uint8_t irqline)
{
    uint16_t port;
    /* if irqline >= 8, the irq corresponds to the slave */
    if(irqline >= 8)
    {
        /* target the slave and subtract 8 */
        port = PIC_SLAVE_PORT_DATA;
        irqline -= 8;
    }
    else
    {
        port = PIC_MASTER_PORT_DATA;
    }

    uint8_t mask = port_inb(port);
    port_outb(mask | (1 << irqline), port);
}

void pic8259_unmask_irq_line(uint8_t irqline)
{
    uint16_t port;
    /* if irqline >= 8, the irq corresponds to the slave */
    if(irqline >= 8)
    {
        /* target the slave and subtract 8 */
        port = PIC_SLAVE_PORT_DATA;
        irqline -= 8;
    }
    else
    {
        port = PIC_MASTER_PORT_DATA;
    }

    uint8_t mask = port_inb(port);
    port_outb(mask & ~(1 << irqline), port);
}

void pic8259_set_mask(uint8_t mask, uint8_t pic)
{
    switch(pic)
    {
    case 0:
        port_outb(mask, PIC_MASTER_PORT_DATA);
        break;
    
    default:
        port_outb(mask, PIC_SLAVE_PORT_DATA);
        break;
    }
}

uint8_t pic8259_get_isr(uint8_t pic)
{
    uint8_t ret;
    switch(pic)
    {
    case 0:
        port_outb(PIC_OWC3_READ_ISR, PIC_MASTER_PORT_COMMAND);
        ret = port_inb(PIC_MASTER_PORT_COMMAND);
        break;

    default: /* default to the first slave */
        port_outb(PIC_OWC3_READ_ISR, PIC_SLAVE_PORT_COMMAND);
        ret = port_inb(PIC_SLAVE_PORT_COMMAND);
        break;
    }
    return ret;
}

uint8_t pic8259_get_irr(uint8_t pic)
{
    uint8_t ret;
    switch(pic)
    {
    case 0:
        port_outb(PIC_OWC3_READ_IRR, PIC_MASTER_PORT_COMMAND);
        ret = port_inb(PIC_MASTER_PORT_COMMAND);
        break;

    default: /* default to the first slave */
        port_outb(PIC_OWC3_READ_IRR, PIC_SLAVE_PORT_COMMAND);
        ret = port_inb(PIC_SLAVE_PORT_COMMAND);
        break;
    }
    return ret;
}

void pic8259_signal_eoi(uint8_t pic)
{
    switch(pic)
    {
    case 0:
        port_outb(0x20, PIC_MASTER_PORT_COMMAND);
        break;
    
    default:
        port_outb(0x20, PIC_MASTER_PORT_COMMAND);
        port_outb(0x20, PIC_SLAVE_PORT_COMMAND);
        break;
    }
}

void pic8259_disable(uint8_t pic)
{
    switch(pic)
    {
    case 0:
        port_outb(0xFF, PIC_MASTER_PORT_DATA);
        break;
    default:
        port_outb(0xFF, PIC_SLAVE_PORT_DATA);
        break;
    }
}

uint8_t pic8259_get_pics_count()
{
    return 2;
}

uint16_t pic8259_get_mask()
{
    uint16_t mask;
    mask = port_inb(PIC_SLAVE_PORT_DATA);
    mask <<= 8;
    mask |= port_inb(PIC_MASTER_PORT_DATA);
    return mask;
}
