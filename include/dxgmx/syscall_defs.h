/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXMGX_SYSCALL_DEFS_H
#define _DXMGX_SYSCALL_DEFS_H

#define SYS_EXIT 0
#define SYS_OPEN 1
#define SYS_READ 2
#define SYS_EXECVE 3

#ifdef _KERNEL

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/syscall_types.h>

/* This file is included once in the kernel in kernel/syscalls.c. DO NOT
 * include this file anywhere else in the kernel. Userspace is free to do
 * whatever. */

SYSCALL_VOID_1(sys_exit, int);
SYSCALL_RETV_3(int, sys_open, const char*, int, mode_t);
SYSCALL_RETV_3(ssize_t, sys_read, int, void*, size_t);
SYSCALL_RETV_3(int, sys_execve, const char*, const char**, const char**);

#endif // _KERNEL

#endif // !_DXMGX_SYSCALL_DEFS_H