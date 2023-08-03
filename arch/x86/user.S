/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#define _ASM
#include <dxgmx/x86/gdt.h>
#include <dxgmx/errno.h>

/* This way of checking for userspace faults in kernel space may have been appropriated from linux */
.section .useraccess, "a"
.local user_copy_to_end

.global user_copy_to
.type user_copy_to, @function
user_copy_to:
    push %ebp
    mov %esp, %ebp
    push %ebx # ebx is the only gp reg we use that is not caller saved

    xor %ecx, %ecx
    movl 0x8(%ebp), %ebx # Get dest from stack
    movl 0xc(%ebp), %edx # Get src from stack

    # We only do 1 byte at a time... oh well
1:
    # Copy, maybe faulting
    mov (%edx), %eax
    movb %al, (%ebx)

    inc %ecx
    inc %ebx
    inc %edx
    cmp 0x10(%ebp), %ecx
    jl 1b

    # Everything was fine
    xor %eax, %eax
user_copy_to_end:
    pop %ebx
    pop %ebp
    ret

.global user_access_fault_stub
.type user_access_fault_stub, @function
user_access_fault_stub:
    mov $-EFAULT, %eax
    jmp user_copy_to_end

.section .text
.global user_jump2user_arch
.type user_jump2user_arch, @function
user_jump2user_arch:

#if defined(_X86_)
    movl 0x4(%esp), %ebx  # The instruction pointer
    movl 0x8(%esp), %ecx  # The stack pointer
#endif // _X86_

    # Move the ring 3 data segment into all segment registers.
    # ss will be handled by iret.
    mov $GDT_USER_DS | 3, %ax  
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    # Set up iret stackframe. ring 3 data (ss) -> esp -> flags -> ring3 code -> eip
    pushl %eax
    pushl %ecx
    pushf
    pushl $GDT_USER_CS | 3
    pushl %ebx
    iret
