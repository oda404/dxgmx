/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#define _ASM
#include <dxgmx/x86/gdt.h>
#include <dxgmx/errno.h>

/* This way of checking for userspace faults in kernel space may have been appropriated from linux */
.section .useraccess, "a"
user_access_end:
    pop %ebx
    pop %ebp
    ret

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
    cmp 0x10(%ebp), %ecx
    jae 2f

    # Copy, maybe faulting
    mov (%edx), %eax
    movb %al, (%ebx)

    inc %ecx
    inc %ebx
    inc %edx
    jmp 1b

2:
    # Everything was fine
    xor %eax, %eax
    jmp user_access_end   

.global user_access_fault_stub
.type user_access_fault_stub, @function
user_access_fault_stub:
    mov $-EFAULT, %eax
    jmp user_access_end

.global user_copy_str_from
.type user_copy_str_from, @function
user_copy_str_from:
    push %ebp
    mov %esp, %ebp
    push %ebx

    # End right now if maxn is 0
    cmpl $0, 0x10(%ebp)
    je loop_end

    xor %ecx, %ecx
    movl 0x8(%ebp), %ebx # Get src from stack
    movl 0xc(%ebp), %edx # Get dest from stack

1:
    movl (%ebx), %eax
    movb %al, (%edx)

    # End early if we hit the null terminator
    cmp $0, %al
    je loop_end

    inc %ebx
    inc %edx
    inc %ecx
    cmp 0x10(%ebp), %ecx
    jl 1b

    # If we got here we hit maxn, and we don't have a null terminator
    dec %edx
    movb $0, (%edx)

loop_end:
    xor %eax, %eax
    jmp user_access_end


.global user_copy_from
.type user_copy_from, @function
user_copy_from:
    push %ebp
    mov %esp, %ebp
    push %ebx # ebx is the only gp reg we use that is not caller saved

    xor %ecx, %ecx
    movl 0x8(%ebp), %ebx # Get src from stack
    movl 0xc(%ebp), %edx # Get dest from stack

    # We only do 1 byte at a time... oh well
1:
    cmp 0x10(%ebp), %ecx
    jae 2f

    # Copy, maybe faulting
    mov (%ebx), %eax
    movb %al, (%edx)

    inc %ecx
    inc %ebx
    inc %edx
    jmp 1b

2:
    # Everything was fine
    xor %eax, %eax
    jmp user_access_end

.global user_strnlen
.type user_strnlen, @function
user_strnlen:
    push %ebp
    mov %esp, %ebp
    push %ebx

    xor %ecx, %ecx
    movl 0x8(%ebp), %ebx # Get str from stack

1:
    # Finish if we hit the maxn
    cmp 0xc(%ebp), %ecx
    jae 2f

    movl (%ebx), %edx

    # End early if we hit the null terminator
    cmp $0, %dl
    je 2f

    inc %ebx
    inc %ecx
    jmp 1b

2:
    mov %ecx, %eax
    jmp user_access_end

.section .text
.global user_jump2user_arch
.type user_jump2user_arch, @function
user_jump2user_arch:

#if defined(CONFIG_X86)
    movl 0x4(%esp), %ebx  # The instruction pointer
    movl 0x8(%esp), %ecx  # The stack pointer
#endif // CONFIG_X86

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
