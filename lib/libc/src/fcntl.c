/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <fcntl.h>
#include <syscalls.h>

int open(const char* path, int flags, mode_t mode)
{
    return syscall3(SYSCALL_OPEN, (syscall_arg_t)path, flags, mode);
}
