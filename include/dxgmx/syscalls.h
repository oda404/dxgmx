/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_SYSCALLS_H
#define _DXGMX_SYSCALLS_H

#include <dxgmx/attrs.h>
#include <dxgmx/types.h>

typedef u32 syscall_t;

#if defined(_X86_) // should really be something like DXGMX_32BITS bits
typedef i32 syscall_ret_t;
#endif

_INIT int syscalls_init();
_INIT int syscalls_arch_init();

syscall_ret_t syscalls_do_handle(syscall_t sysn, ...);

#define SYSCALL_EXIT 1
#define SYSCALL_READ 2

/* Actual system calls. */
void syscall_exit(int status);
ssize_t syscall_read(int fd, void* buf, size_t n);

#endif // !_DXGMX_SYSCALLS_H
