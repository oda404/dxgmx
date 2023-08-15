/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/syscall_defs.h>
#include <dxgmx/syscalls.h>
#include <posix/sys/types.h>
#include <stdarg.h>

static _RO_POST_INIT SyscallEntry* g_syscall_entries;
static _RO_POST_INIT size_t g_syscall_entry_count;

static int sys_undefined(syscall_t sysn)
{
    klogln(WARN, "syscalls: Invalid syscall number: 0x%X!", sysn);
    return -ENOSYS;
}

syscall_ret_t syscalls_do_handle(syscall_t n, ...)
{
    if (n >= g_syscall_entry_count)
        return sys_undefined(n);

    va_list list;
    va_start(list, n);
    return g_syscall_entries[n].func(list);
    /* The syscall entry calls va_end on list, since it may not return at all */
}

int syscalls_init()
{
    extern _INIT int syscalls_arch_init();
    int st = syscalls_arch_init();
    if (st < 0)
        panic("Failed to initialize syscalls.");

    g_syscall_entries = (SyscallEntry*)kimg_syscalls_start();
    g_syscall_entry_count =
        (kimg_syscalls_end() - kimg_syscalls_start()) / sizeof(SyscallEntry);

    return 0;
}
