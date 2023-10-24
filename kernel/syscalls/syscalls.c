/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/generated/syscall_defs.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/syscalls.h>
#include <dxgmx/types.h>
#include <stdarg.h>

static _RO_POST_INIT SyscallEntry* g_syscall_entries;
static _RO_POST_INIT size_t g_syscall_entry_count;

int sys_undefined(syscall_t sysn)
{
    klogln(WARN, "syscalls: Called invalid syscall number 0x%X!", sysn);
    return -ENOSYS;
}

syscall_ret_t syscalls_do_handle(syscall_t n, ...)
{
    if (n >= g_syscall_entry_count)
        return sys_undefined(n);

    va_list list;
    va_start(list, n);
    return g_syscall_entries[n].func(&list);
    /* NOTE: The syscall entry calls va_end on list, since it may not return at
     * all. The man page for va_end says that va_end MUST be called within the
     * same function that called va_start. I went down a rabbit hole trying to
     * find examples of ABIs where this would actually be a MUST but I came out
     * empty handed. Moreover 99.9% (and 100% of sane) implementations (that I
     * could find) actually define va_end as either a noop or a simple
     * asignment. Even if we ever hit an obscure case where va_end would need to
     * be called inside this function, I don't think I'd like to support such
     * platforms, at least without a solid argument for doing so. That being
     * said, until further notice we have (un)defined behaviour in the syscall
     * layer. */
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
