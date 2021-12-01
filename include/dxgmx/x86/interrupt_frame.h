/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_INTERRUPT_FRAME_H
#define _DXGMX_X86_INTERRUPT_FRAME_H

#include<dxgmx/compiler_attrs.h>
#include<dxgmx/types.h>

typedef struct 
_ATTR_PACKED S_InterruptFrame
{
    u32 edi;
    u32 esi;
    u32 ebp;
    u32 esp;
    u32 ebx;
    u32 edx;
    u32 ecx;
    u32 eax;

    u32 code;

    u32 eip;
    u32 cs;
    u32 eflags;
} InterruptFrame;


#endif //_DXGMX_X86_INTERRUPT_FRAME_H
