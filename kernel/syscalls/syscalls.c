/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/syscall_numbers.h>
#include <dxgmx/syscalls.h>
#include <posix/sys/types.h>
#include <stdarg.h>

/* Actual system calls. */
extern void syscall_exit(int status);
extern int syscall_open(const char* path, int flags, mode_t mode);
extern ssize_t syscall_read(int fd, void* buf, size_t n);
extern int
syscall_execve(const char* path, const char* argv[], const char* envv[]);
extern void syscall_dxgmx_log(const char* fmt, ...);

/* No such syscall */
static int syscall_undefined(syscall_t sysn)
{
    klogln(WARN, "Invalid syscall number: 0x%X!", sysn);
    return -ENOSYS;
}

static syscall_ret_t syscalls_do_handle(syscall_t n, ...)
{
    va_list list;
    va_start(list, n);

    syscall_ret_t ret = 0;

    switch (n)
    {
    case SYSCALL_EXIT:
        syscall_exit(va_arg(list, int));
        break;

    case SYSCALL_OPEN:
        ret = syscall_open(
            va_arg(list, const char*), va_arg(list, int), va_arg(list, mode_t));
        break;

    case SYSCALL_READ:
        ret = syscall_read(
            va_arg(list, int), va_arg(list, void*), va_arg(list, size_t));
        break;

    case SYSCALL_EXECVE:
        ret = syscall_execve(
            va_arg(list, const char*),
            va_arg(list, const char**),
            va_arg(list, const char**));
        break;

    case SYSCALL_DXGMX_LOG:
        syscall_dxgmx_log(va_arg(list, const char*), list);
        break;

    default:
        ret = syscall_undefined(n);
        break;
    }

    va_end(list);

    return ret;
}

_INIT int syscalls_arch_init(syscall_ret_t (*)(syscall_t, ...));

int syscalls_init()
{
    return syscalls_arch_init(syscalls_do_handle);
}
