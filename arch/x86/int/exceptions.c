/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/panic.h>
#include <dxgmx/todo.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/interrupt_frame.h>

static void divbyzero_isr(const InterruptFrame* frame, const void* data)
{
    (void)data;

    panic(
        "Division by zero in ring 0 at EIP: 0x%p. Not proceeding.",
        (void*)frame->eip);
}

static void debug_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;
    TODO();
}

static void overflow_isr(const InterruptFrame* frame, const void* data)
{
    (void)data;
    klogln(
        WARN,
        "Overflow in ring 0 at EIP: 0x%p, EFLAGS: 0x%p.",
        (void*)frame->eip,
        (void*)frame->eflags);
}

static void
boundrange_exceeded_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;
    TODO_FATAL();
}

static void invalid_opcode_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;
    panic(
        "Invalid instruction at EIP: 0x%p. Not proceeding.", (void*)frame->eip);
}

static void fpu_not_available_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;
    TODO_FATAL();
}

static void double_fault_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    panic(
        "Double fault! EIP: 0x%p. Obviously not proceeding.",
        (void*)frame->eip);
}

static void invalid_tss_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void segment_absent_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void stack_seg_fault(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void
general_prot_fault_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void
x87floating_point_err_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void alignment_check_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void machine_check_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    panic(
        "You just got machine checked, EIP: 0x%p. Not proceeding",
        (void*)frame->eip);
}

static void
simdfloating_point_err_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void virt_err_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

static void security_err_isr(const InterruptFrame* frame, const void* data)
{
    (void)frame;
    (void)data;

    TODO_FATAL();
}

_INIT void exceptions_set_up_common_handlers()
{
    idt_register_isr(TRAP0, divbyzero_isr);
    idt_register_isr(TRAP1, debug_isr);
    idt_register_isr(TRAP4, overflow_isr);
    idt_register_isr(TRAP5, boundrange_exceeded_isr);
    idt_register_isr(TRAP6, invalid_opcode_isr);
    idt_register_isr(TRAP7, fpu_not_available_isr);
    idt_register_isr(TRAP8, double_fault_isr);
    idt_register_isr(TRAP10, invalid_tss_isr);
    idt_register_isr(TRAP11, segment_absent_isr);
    idt_register_isr(TRAP12, stack_seg_fault);
    idt_register_isr(TRAP13, general_prot_fault_isr);
    idt_register_isr(TRAP16, x87floating_point_err_isr);
    idt_register_isr(TRAP17, alignment_check_isr);
    idt_register_isr(TRAP18, machine_check_isr);
    idt_register_isr(TRAP19, simdfloating_point_err_isr);
    idt_register_isr(TRAP20, virt_err_isr);
    idt_register_isr(TRAP30, security_err_isr);
}
