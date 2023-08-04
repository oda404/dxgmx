/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/syscall_defs.h>
#include <syscalls.h>

void exit(int status)
{
    (void)syscall1(SYS_EXIT, status);
}
