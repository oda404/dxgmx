
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/portio.h>

/** 
 * Because of ancient computers the switch for disabling
 * the NMIs is the 7th bit of the CMOS register port, 0x70.
 * If this bit is set NMIs will be masked(ignored).
*/

void cmos_disable_nmi()
{
    port_outb(
        port_inb(CMOS_PORT_REG) | (1 << 7), 
        CMOS_PORT_REG
    );
}

void cmos_enable_nmi()
{
    port_outb(
        port_inb(CMOS_PORT_REG) & ~(1 << 7), 
        CMOS_PORT_REG
    );
}

int cmos_is_nmi_enabled()
{
    return port_inb(CMOS_PORT_REG) & (1 << 7);
}

uint8_t cmos_port_inb(uint8_t port)
{
    port_outb(port, CMOS_PORT_REG);
    return port_inb(CMOS_PORT_DATA);
}
