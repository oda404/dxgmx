
#ifndef _DXGMX_X86_CMOS_H
#define _DXGMX_X86_CMOS_H

#include<stdint.h>

#define CMOS_PORT_REG      0x70
#define CMOS_PORT_DATA     0x71

void cmos_disable_nmi();
void cmos_enable_nmi();
int cmos_is_nmi_enabled();
uint8_t cmos_port_inb(uint8_t port);

#endif //_DXGMX_X86_CMOS_H
