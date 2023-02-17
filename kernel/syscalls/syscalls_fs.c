/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/fs/vfs.h>
#include <dxgmx/klog.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/todo.h>
#include <dxgmx/user.h>

int syscall_open(const char* path, int flags, mode_t mode)
{
    return vfs_open(path, flags, mode, sched_current_proc());
}

ssize_t syscall_read(int fd, void* buf, size_t n)
{
    return vfs_read(fd, buf, n, sched_current_proc());
}
