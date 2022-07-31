/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <syscalls.h>

void exit(int status)
{
    (void)syscall1(1, status);
}
