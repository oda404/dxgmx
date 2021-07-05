/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_PORTIO_H
#define _DXGMX_X86_PORTIO_H

#include<dxgmx/attrs.h>
#include<stdint.h>

_ATTR_ALWAYS_INLINE
void port_outb(uint8_t value, uint16_t port)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

_ATTR_ALWAYS_INLINE 
uint8_t port_inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif // _DXGMX_X86_PORTIO_H
