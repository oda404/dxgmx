/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/x86/interrupts.h>

/** 
 * Because of ancient computers the switch for disabling
 * the NMIs is the 7th bit of the CMOS register port, 0x70.
 * If this bit is set NMIs will be masked(ignored).
*/

static u8 g_nmienabled = 1;

void cmos_disable_nmi()
{
    g_nmienabled = 0;
    port_outb(0x8D, CMOS_PORT_REG);
    port_inb(CMOS_PORT_DATA);
}

void cmos_enable_nmi()
{
    g_nmienabled = 1;
    port_outb(0x0D, CMOS_PORT_REG);
    port_inb(CMOS_PORT_DATA);
}

u8 cmos_port_inb(u8 port, int nmistate)
{
    interrupts_disable();

    port_outb(port | 0x80, CMOS_PORT_REG);
    u8 ret = port_inb(CMOS_PORT_DATA);

    switch(nmistate)
    {
    case NMIDISABLED:
        break;

    case NMIENABLED:
        cmos_enable_nmi();
        break;

    case NMIKEEP:
        if(g_nmienabled)
            cmos_enable_nmi();
        break;
    }

    interrupts_enable();
    
    return ret;
}

void cmos_port_outb(u8 value, u8 port, int nmistate)
{
    interrupts_disable();

    port_outb(port | 0x80, CMOS_PORT_REG);
    port_outb(value, CMOS_PORT_DATA);

    switch(nmistate)
    {
    case NMIDISABLED:
        break;

    case NMIENABLED:
        cmos_enable_nmi();
        break;

    case NMIKEEP:
        if(g_nmienabled)
            cmos_enable_nmi();
        break;
    }

    interrupts_enable();
}
