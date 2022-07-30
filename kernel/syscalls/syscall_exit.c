/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/todo.h>

void syscall_exit(int status)
{
    klogln(DEBUG, "exit(%d)", status);
    TODO_FATAL();
}
