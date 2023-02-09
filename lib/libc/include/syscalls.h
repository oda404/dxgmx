/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _LIBC_SYSCALLS_H
#define _LIBC_SYSCALLS_H

#include <dxgmx/syscall_numbers.h>
#include <stdint.h>

typedef uint32_t syscall_arg_t;
typedef syscall_arg_t syscall_ret_t;

syscall_ret_t syscall0(syscall_arg_t sysn);

syscall_ret_t syscall1(syscall_arg_t sysn, syscall_arg_t a1);

syscall_ret_t syscall2(syscall_arg_t sysn, syscall_arg_t a1, syscall_arg_t a2);

syscall_ret_t syscall3(
    syscall_arg_t sysn, syscall_arg_t a1, syscall_arg_t a2, syscall_arg_t a3);

syscall_ret_t syscall4(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4);

syscall_ret_t syscall5(
    syscall_arg_t sysn,
    syscall_arg_t a1,
    syscall_arg_t a2,
    syscall_arg_t a3,
    syscall_arg_t a4,
    syscall_arg_t a5);

#endif // !_LIBC_SYSCALLS_H
