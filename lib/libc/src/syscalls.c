/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <syscalls.h>

syscall_ret_t syscall0(syscall_arg_t sysn)
{
    syscall_ret_t ret;

    __asm__ volatile("int $0x80"
                     : "=a"(ret)
                     : "a"(sysn)
                     : "memory", "cc", "ebx", "ecx", "edx", "esi", "edi");

    return ret;
}

syscall_ret_t syscall1(syscall_arg_t sysn, syscall_arg_t a1)
{
    syscall_ret_t ret;

    __asm__ volatile("int $0x80"
                     : "=a"(ret), "+b"(a1)
                     : "a"(sysn)
                     : "memory", "cc", "ecx", "edx", "esi", "edi");

    return ret;
}

syscall_ret_t syscall2(syscall_arg_t sysn, syscall_arg_t a1, syscall_arg_t a2)
{
    syscall_ret_t ret;

    __asm__ volatile("int $0x80"
                     : "=a"(ret), "+b"(a1), "+c"(a2)
                     : "a"(sysn)
                     : "memory", "cc", "edx", "esi", "edi");

    return ret;
}

syscall_ret_t syscall3(
    syscall_arg_t sysn, syscall_arg_t a1, syscall_arg_t a2, syscall_arg_t a3)
{
    syscall_ret_t ret;

    __asm__ volatile("int $0x80"
                     : "=a"(ret), "+b"(a1), "+c"(a2), "+d"(a3)
                     : "a"(sysn)
                     : "memory", "cc", "esi", "edi");

    return ret;
}

syscall_ret_t syscall4(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4)
{
    syscall_ret_t ret;

    __asm__ volatile("int $0x80"
                     : "=a"(ret), "+b"(a1), "+c"(a2), "+d"(a3), "+S"(a4)
                     : "a"(sysn)
                     : "memory", "cc", "edi");

    return ret;
}

syscall_ret_t syscall5(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4,
    syscall_arg_t a5)
{
    syscall_ret_t ret;

    __asm__ volatile(
        "int $0x80"
        : "=a"(ret), "+b"(a1), "+c"(a2), "+d"(a3), "+S"(a4), "+D"(a5)
        : "a"(sysn)
        : "memory", "cc");

    return ret;
}
