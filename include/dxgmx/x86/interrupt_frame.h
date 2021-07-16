

#ifndef _DXGMX_X86_INTERRUPT_FRAME_H
#define _DXGMX_X86_INTERRUPT_FRAME_H

#include<dxgmx/attrs.h>
#include<stdint.h>

typedef struct 
_ATTR_PACKED S_InterruptFrame
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint16_t code;

    uint32_t eip;
    uint16_t padding;
    uint16_t cs;
    uint32_t eflags;
} InterruptFrame;


#endif //_DXGMX_X86_INTERRUPT_FRAME_H
