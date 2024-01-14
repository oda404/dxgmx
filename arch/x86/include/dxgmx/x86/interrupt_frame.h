/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_INTERRUPT_FRAME_H
#define _DXGMX_X86_INTERRUPT_FRAME_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/types.h>

/* Registers names that begin with 'x' instead of 'e'/'r'/'' are registers
common to both x86 and x86_64 that change size/are padded to form either
32bit or 64bit values accordingly. */
typedef struct _ATTR_PACKED S_InterruptFrame
{
#ifdef CONFIG_64BIT
    /* Only r8-r11 are scratch registers, r12-r15 are preserved */
    u64 r11;
    u64 r10;
    u64 r9;
    u64 r8;
#endif // !CONFIG_64BIT

    /* There are registers here we could omit here, but they're needed for
    syscalls. Maybe make a special interrupt frame only for syscalls?. The stack
    pointer for example is not saved *here* at all since it's preserved and we
    don't use it for syscalls */
    size_t xdi;
    size_t xsi;
    size_t xbp;
    size_t xbx;
    size_t xdx;
    size_t xcx;
    size_t xax;

    /* pushed for some exceptions, a dummy 0 will be pushed for the rest */
    size_t code;

    /* CPU-pushed */
    size_t xip;
    size_t xcs;
    size_t xflags;

    /* The CPU might actually push more data here that we don't care about
    (right now at least).
    On x86_64 it will push ss:rsp.
    On x86 it will push ss:esp if it's a cross privilege interrupt.
    The CPU will of course know what to pop off the stack on iret, regardless of
    this struct. This structure should only by accesed as an InterruptFrame*
    since the actual size of what the CPU pushes will vary. */
} InterruptFrame;

#endif //_DXGMX_X86_INTERRUPT_FRAME_H
