/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/todo.h>

ssize_t syscall_read(int fd, void* buf, size_t n)
{
    klogln(DEBUG, "read(%d, %p, %X)", fd, buf, n);
    TODO_FATAL();
}
