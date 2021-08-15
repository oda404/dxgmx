/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

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
} NMIState;

void cmos_disable_nmi();
void cmos_enable_nmi();
/** 
 * Safe way to read from a cmos register.
 * The parameter 'nmistate' is what the NMI state will be
 * after the value is read, see enum E_NMIState.
*/
uint8_t cmos_port_inb(uint8_t port, int nmistate);
/** 
 * Safe way to write to a cmos register.
 * The parameter 'nmistate' is what the NMI state will be
 * after the value is written, see enum E_NMIState.
*/
void cmos_port_outb(uint8_t value, uint8_t port, int nmistate);

#endif //_DXGMX_X86_CMOS_H
