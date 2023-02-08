/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#define _ASM
#include <dxgmx/x86/gdt.h>

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
