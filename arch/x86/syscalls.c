/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/syscalls.h>
#include <dxgmx/x86/idt.h>

static void x86syscall_isr(InterruptFrame* frame)
{
    /* Return value of syscall goes in eax. Note that even if the syscall
     * returns void, we still return and set eax to 0. Should we fix this ? */
    frame->eax = syscalls_do_handle(
        frame->eax,
        frame->ebx,
        frame->ecx,
        frame->edx,
        frame->esi,
        frame->edi,
        frame->ebp);
}

int syscalls_arch_init()
{
    idt_register_trap_isr(0x80, 3, x86syscall_isr);
    return 0;
}
