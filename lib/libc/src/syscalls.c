/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <syscalls.h>

#define SYSCALL_ATTRS __attribute__((naked)) __attribute__((cdecl))

SYSCALL_ATTRS syscall_ret_t syscall0(syscall_arg_t sysn)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %ebp               \n"
                     "ret");
}

SYSCALL_ATTRS syscall_ret_t syscall1(syscall_arg_t sysn, syscall_arg_t a1)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "push %ebx              \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "movl 0xc(%ebp),  %ebx  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %ebx               \n"
                     "pop %ebp               \n"
                     "ret");
}

SYSCALL_ATTRS syscall_ret_t
syscall2(syscall_arg_t sysn, syscall_arg_t a1, syscall_arg_t a2)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "push %ebx              \n"
                     "push %ecx              \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "movl 0xc(%ebp),  %ebx  \n"
                     "movl 0x10(%ebp), %ecx  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %ecx               \n"
                     "pop %ebx               \n"
                     "pop %ebp               \n"
                     "ret");
}

SYSCALL_ATTRS syscall_ret_t syscall3(
    syscall_arg_t sysn, syscall_arg_t a1, syscall_arg_t a2, syscall_arg_t a3)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "push %ebx              \n"
                     "push %ecx              \n"
                     "push %edx              \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "movl 0xc(%ebp),  %ebx  \n"
                     "movl 0x10(%ebp), %ecx  \n"
                     "movl 0x14(%ebp), %edx  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %edx               \n"
                     "pop %ecx               \n"
                     "pop %ebx               \n"
                     "pop %ebp               \n"
                     "ret");
}

SYSCALL_ATTRS syscall_ret_t syscall4(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "push %ebx              \n"
                     "push %ecx              \n"
                     "push %edx              \n"
                     "push %esi              \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "movl 0xc(%ebp),  %ebx  \n"
                     "movl 0x10(%ebp), %ecx  \n"
                     "movl 0x14(%ebp), %edx  \n"
                     "movl 0x18(%ebp), %esi  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %esi               \n"
                     "pop %edx               \n"
                     "pop %ecx               \n"
                     "pop %ebx               \n"
                     "pop %ebp               \n"
                     "ret");
}

SYSCALL_ATTRS syscall_ret_t syscall5(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4,
    syscall_arg_t a5)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "push %ebx              \n"
                     "push %ecx              \n"
                     "push %edx              \n"
                     "push %esi              \n"
                     "push %edi              \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "movl 0xc(%ebp),  %ebx  \n"
                     "movl 0x10(%ebp), %ecx  \n"
                     "movl 0x14(%ebp), %edx  \n"
                     "movl 0x18(%ebp), %esi  \n"
                     "movl 0x1c(%ebp), %edi  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %edi               \n"
                     "pop %esi               \n"
                     "pop %edx               \n"
                     "pop %ecx               \n"
                     "pop %ebx               \n"
                     "pop %ebp               \n"
                     "ret");
}

SYSCALL_ATTRS syscall_ret_t syscall6(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4,
    syscall_arg_t a5,
    syscall_arg_t a6)
{
    __asm__ volatile("push %ebp              \n"
                     "mov  %esp, %ebp        \n"
                     "\n"
                     "push %ebx              \n"
                     "push %ecx              \n"
                     "push %edx              \n"
                     "push %esi              \n"
                     "push %edi              \n"
                     "\n"
                     "movl 0x8(%ebp),  %eax  \n"
                     "movl 0xc(%ebp),  %ebx  \n"
                     "movl 0x10(%ebp), %ecx  \n"
                     "movl 0x14(%ebp), %edx  \n"
                     "movl 0x18(%ebp), %esi  \n"
                     "movl 0x1c(%ebp), %edi  \n"
                     "movl 0x20(%ebp), %ebp  \n"
                     "\n"
                     "int $0x80              \n"
                     "\n"
                     "pop %edi               \n"
                     "pop %esi               \n"
                     "pop %edx               \n"
                     "pop %ecx               \n"
                     "pop %ebx               \n"
                     "pop %ebp               \n"
                     "ret");
}
