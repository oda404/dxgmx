/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/mem/pagefault.h>
#include <dxgmx/x86/idt.h>

#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))
#define PAGEFAULT_IS_EXEC(x) (x & (1 << 4))

static void pagefault_isr(InterruptFrame* frame)
{
    const ptr faultaddr = cpu_read_cr2();

    PageFaultReason reason;
    if (PAGEFAULT_IS_PROT_VIOL(frame->code))
        reason = PAGEFAULT_REASON_PROT_FAULT;
    else
        reason = PAGEFAULT_REASON_ABSENT;

    PageFaultAction action;
    if (PAGEFAULT_IS_EXEC(frame->code))
        action = PAGEFAULT_ACTION_EXEC;
    else if (PAGEFAULT_IS_WRITE(frame->code))
        action = PAGEFAULT_ACTION_WRITE;
    else
        action = PAGEFAULT_ACTION_READ;

    frame->eip =
        pagefault_handle(faultaddr, frame->eip, frame->cs & 3, reason, action);
}

_INIT void pagefault_setup_arch()
{
    idt_register_trap_isr(TRAP_PAGEFAULT, 0, pagefault_isr);
}
