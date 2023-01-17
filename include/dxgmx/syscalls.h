/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_SYSCALLS_H
#define _DXGMX_SYSCALLS_H

#include <dxgmx/attrs.h>
#include <dxgmx/types.h>

#define SYSCALL_EXIT 1
#define SYSCALL_READ 2
#define SYSCALL_EXECVE 3

typedef u32 syscall_t;

#if defined(_X86_) // FIXME: should really be something like DXGMX_32BITS bits
/* Returns type of syscalls */
typedef u32 syscall_arg_t;
typedef i32 syscall_ret_t;
#endif

/* Syscall handler callback */
typedef syscall_ret_t (*syscall_handler_t)(syscall_t sysn, ...);

/* Initialize system calls */
_INIT int syscalls_init();

#endif // !_DXGMX_SYSCALLS_H
