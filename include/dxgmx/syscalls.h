/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_SYSCALLS_H
#define _DXGMX_SYSCALLS_H

#include <dxgmx/attrs.h>
#include <dxgmx/syscall_types.h>

/* Initialize system calls */
_INIT int syscalls_init();

/* Global syscall handler. Called by the arch specific syscall ISR. */
syscall_ret_t syscalls_do_handle(syscall_t n, ...);

#endif // !_DXGMX_SYSCALLS_H
