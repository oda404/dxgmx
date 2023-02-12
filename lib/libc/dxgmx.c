/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx.h>
#include <stdarg.h>
#include <syscalls.h>

void dxgmx_klog(const char* fmt, ...)
{
    va_list list;
    va_start(list, fmt);

    syscall2(SYSCALL_DXGMX_LOG, (syscall_arg_t)fmt, (syscall_arg_t)list);

    va_end(list);
}
