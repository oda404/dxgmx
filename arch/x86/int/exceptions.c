/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/panic.h>
#include <dxgmx/todo.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/interrupt_frame.h>

static void divbyzero_isr(InterruptFrame* frame)
{

    panic(
        "Division by zero in ring 0 at xip: 0x%p. Not proceeding.",
        (void*)frame->xip);
}

static void debug_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO();
}

static void breakpoint_isr(InterruptFrame* frame)
{
    klogln(WARN, "Breakpoint in ring 0, ip: 0x%p", (void*)frame->xip);
}

static void overflow_isr(InterruptFrame* frame)
{

    klogln(
        WARN,
        "Overflow in ring 0 at xip: 0x%p, xflags: 0x%p.",
        (void*)frame->xip,
        (void*)frame->xflags);
}

static void boundrange_exceeded_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void invalid_opcode_isr(InterruptFrame* frame)
{
    (void)frame;

    panic(
        "Invalid instruction at xip: 0x%p. Not proceeding.", (void*)frame->xip);
}

static void fpu_not_available_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void double_fault_isr(InterruptFrame* frame)
{
    (void)frame;

    panic(
        "Double fault! xip: 0x%p. Obviously not proceeding.",
        (void*)frame->xip);
}

static void invalid_tss_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void segment_absent_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void stack_seg_fault(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void general_prot_fault_isr(InterruptFrame* frame)
{
    if ((frame->xcs & 3) == 0)
    {
        panic(
            "#GP in ring 0 at 0x%p, code: 0x%zx. Not proceeding.",
            (void*)frame->xip,
            frame->code);
    }

    klogln(
        WARN,
        "#GP in ring 3 at 0x%p, code: 0x%zx.",
        (void*)frame->xip,
        frame->code);

    TODO_FATAL();
}

static void pagefault_isr(InterruptFrame*)
{
    panic("Page fault before the actual pagefault handler was installed!");
}

static void x87floating_point_err_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void alignment_check_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void machine_check_isr(InterruptFrame* frame)
{
    (void)frame;

    panic(
        "You just got machine checked, xip: 0x%p. Not proceeding",
        (void*)frame->xip);
}

static void simdfloating_point_err_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void virt_err_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

static void control_prot_isr(InterruptFrame* frame)
{
    klogln(WARN, "Control protection exception, ring: %zu", frame->xcs & 3);
}

static void hypervisor_injection_exception_isr(InterruptFrame* frame)
{
    klogln(WARN, "Hypervisor injection exception, ring: %zu", frame->xcs & 3);
}

static void vmm_comm_exception_isr(InterruptFrame* frame)
{
    klogln(WARN, "VMM communication exception, ring: %zu", frame->xcs & 3);
}

static void security_err_isr(InterruptFrame* frame)
{
    (void)frame;

    TODO_FATAL();
}

_INIT void exceptions_set_up_common_handlers()
{
    idt_register_trap_isr(TRAP_DIV_ERR, 0, divbyzero_isr);
    idt_register_trap_isr(TRAP_DEBUG, 0, debug_isr);
    idt_register_trap_isr(TRAP_BREAKPOINT, 0, breakpoint_isr);
    idt_register_trap_isr(TRAP_OVERFLOW, 0, overflow_isr);
    idt_register_trap_isr(TRAP_BOUND_EXCEEDED, 0, boundrange_exceeded_isr);
    idt_register_trap_isr(TRAP_INVALID_OPCODE, 0, invalid_opcode_isr);
    idt_register_trap_isr(TRAP_FPU_NOT_AVAIL, 0, fpu_not_available_isr);
    idt_register_trap_isr(TRAP_DOUBLEFAULT, 0, double_fault_isr);
    idt_register_trap_isr(TRAP_INVALID_TSS, 0, invalid_tss_isr);
    idt_register_trap_isr(TRAP_ABSENT_SEGMENT, 0, segment_absent_isr);
    idt_register_trap_isr(TRAP_SSFAULT, 0, stack_seg_fault);
    idt_register_trap_isr(TRAP_GPF, 0, general_prot_fault_isr);
    idt_register_trap_isr(TRAP_PAGEFAULT, 0, pagefault_isr);
    idt_register_trap_isr(TRAP_X87_FP_EXCEPTION, 0, x87floating_point_err_isr);
    idt_register_trap_isr(TRAP_ALIGNMENT_CHECK, 0, alignment_check_isr);
    idt_register_trap_isr(TRAP_MACHINE_CHECK, 0, machine_check_isr);
    idt_register_trap_isr(
        TRAP_SIMD_FP_EXCEPTION, 0, simdfloating_point_err_isr);
    idt_register_trap_isr(TRAP_VIRT_EXCEPTION, 0, virt_err_isr);
    idt_register_trap_isr(
        TRAP_CONTROL_PROTECTION_EXCEPTION, 0, control_prot_isr);
    idt_register_trap_isr(
        TRAP_HYPERVISOR_EXCEPTION, 0, hypervisor_injection_exception_isr);
    idt_register_trap_isr(TRAP_VMM_COMM_EXCEPTION, 0, vmm_comm_exception_isr);
    idt_register_trap_isr(TRAP_SECURITY_EXCEPTION, 0, security_err_isr);
}
