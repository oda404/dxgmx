
#ifndef __DXGMX_X86_PORTIO_H__
#define __DXGMX_X86_PORTIO_H__

#include<dxgmx/gcc/attrs.h>
#include<stdint.h>

__ATTR_ALWAYS_INLINE
void port_outb(uint8_t value, uint16_t port)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

__ATTR_ALWAYS_INLINE 
uint8_t port_inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif // __DXGMX_X86_PORTIO_H__
