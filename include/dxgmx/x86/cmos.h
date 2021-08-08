
#ifndef _DXGMX_X86_CMOS_H
#define _DXGMX_X86_CMOS_H

#include<stdint.h>

#define CMOS_PORT_REG      0x70
#define CMOS_PORT_DATA     0x71

typedef enum
E_NMIState
{
    /* NMIs are enabled. */
    NMIENABLED  = 0,
    /* NMIs are disabled */
    NMIDISABLED = 1,
    /* NMIs are kept the way they are */
    NMIKEEP     = 2,
    /* NMIs state is manually specified in the 'value' argument. */
    MMIMAN      = 3
} NMIState;

void cmos_disable_nmi();
void cmos_enable_nmi();
int cmos_is_nmi_enabled();
uint8_t cmos_port_inb(uint8_t port, int nmistate);
void cmos_port_outb(uint8_t value, uint8_t port, int nmistate);

#endif //_DXGMX_X86_CMOS_H
