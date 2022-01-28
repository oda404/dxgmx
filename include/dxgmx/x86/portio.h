/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_PORTIO_H
#define _DXGMX_X86_PORTIO_H

#include<dxgmx/compiler_attrs.h>
#include<dxgmx/types.h>

_ATTR_ALWAYS_INLINE
void port_outb(u8 value, u16 port)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

_ATTR_ALWAYS_INLINE 
u8 port_inb(u16 port)
{
    u8 ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

_ATTR_ALWAYS_INLINE
void port_outw(u16 value, u16 port)
{
    __asm__ volatile("outw %0, %1": : "a"(value), "Nd"(port));
}

_ATTR_ALWAYS_INLINE
u16 port_inw(u16 port)
{
    u16 ret;
    __asm__ volatile("inw %1, %0": "=a"(ret) : "Nd"(port));
    return ret;
}

_ATTR_ALWAYS_INLINE
void port_outl(u32 value, u16 port)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

_ATTR_ALWAYS_INLINE 
u32 port_inl(u16 port)
{
    u32 ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif // _DXGMX_X86_PORTIO_H
