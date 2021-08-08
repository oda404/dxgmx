
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/portio.h>
#include<dxgmx/types.h>

/** 
 * Because of ancient computers the switch for disabling
 * the NMIs is the 7th bit of the CMOS register port, 0x70.
 * If this bit is set NMIs will be masked(ignored).
*/

#define NMI_ENABLE(x)  (x & 0x7F)
#define NMI_DISABLE(x) (x | 0x80)

static u8 g_nmienabled = 1;

void cmos_disable_nmi()
{
    g_nmienabled = 0;
    port_outb(NMI_DISABLE(port_inb(CMOS_PORT_REG)), CMOS_PORT_REG);
}

void cmos_enable_nmi()
{
    g_nmienabled = 1;
    port_outb(NMI_ENABLE(port_inb(CMOS_PORT_REG)), CMOS_PORT_REG);
}

int cmos_is_nmi_enabled()
{
    return g_nmienabled;
}

uint8_t cmos_port_inb(uint8_t port, int nmistate)
{
    switch(nmistate)
    {
    case NMIENABLED:
        g_nmienabled = 1;
        port = NMI_ENABLE(port);
        break;

    case NMIDISABLED:
        g_nmienabled = 0;
        port = NMI_DISABLE(port);
        break;

    case NMIKEEP:
        port = (g_nmienabled ? NMI_ENABLE(port) : NMI_DISABLE(port));
        break;

    default:
        break;
    }

    port_outb(port, CMOS_PORT_REG);
    return port_inb(CMOS_PORT_DATA);
}

void cmos_port_outb(uint8_t value, uint8_t port, int nmistate)
{
    switch(nmistate)
    {
    case NMIENABLED:
        g_nmienabled = 1;
        port = NMI_ENABLE(port);
        break;

    case NMIDISABLED:
        g_nmienabled = 0;
        port = NMI_DISABLE(port);
        break;

    case NMIKEEP:
        port = (g_nmienabled ? NMI_ENABLE(port) : NMI_DISABLE(port));
        break;

    default:
        break;
    }

    port_outb(port, CMOS_PORT_REG);
    port_outb(value, CMOS_PORT_DATA);
}
