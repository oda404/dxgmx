/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/syscalls.h>
#include <stdarg.h>

static int syscall_undefined(syscall_t sysn)
{
    klogln(WARN, "Invalid syscall number: 0x%X!", sysn);
    return -ENOSYS;
}

syscall_ret_t syscalls_do_handle(syscall_t n, ...)
{
    va_list list;
    va_start(list, n);

    syscall_ret_t ret = 0;

    switch (n)
    {
    case SYSCALL_EXIT:
        syscall_exit(va_arg(list, int));
        break;

    case SYSCALL_READ:
        ret = syscall_read(
            va_arg(list, int), va_arg(list, void*), va_arg(list, size_t));
        break;

    default:
        ret = syscall_undefined(n);
        break;
    }

    va_end(list);

    return ret;
}

int syscalls_init()
{
    return syscalls_arch_init();
}
