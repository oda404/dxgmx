/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_SYSCALL_TYPES_H
#define _DXGMX_SYSCALL_TYPES_H

#ifdef _KERNEL
#include <dxgmx/types.h>
#else
#include <dxgmx/sys/types_defs.h>
#endif

typedef size_t syscall_t;
typedef size_t syscall_arg_t;
typedef ssize_t syscall_ret_t;

#ifdef _KERNEL
#include <stdarg.h>

typedef struct SyscallEntry
{
    syscall_ret_t (*func)(va_list);
} SyscallEntry;

#define SYSCALL_VOID_1(_name, _arg1)                                           \
    void _name(_arg1);                                                         \
    static syscall_ret_t _g_##_name##_stub(va_list _list)                      \
    {                                                                          \
        _arg1 _1 = va_arg(_list, _arg1);                                       \
        va_end(_list);                                                         \
        _name(_1);                                                             \
        return 0;                                                              \
    }                                                                          \
    _ATTR_SECTION(".syscalls")                                                 \
    _ATTR_USED SyscallEntry _g_##_name##_entry =                               \
        (SyscallEntry){.func = _g_##_name##_stub};

#define SYSCALL_RETV_3(_ret, _name, _arg1, _arg2, _arg3)                       \
    _ret _name(_arg1, _arg2, _arg3);                                           \
    static syscall_ret_t _g_##_name##_stub(va_list _list)                      \
    {                                                                          \
        _arg1 _1 = va_arg(_list, _arg1);                                       \
        _arg2 _2 = va_arg(_list, _arg2);                                       \
        _arg3 _3 = va_arg(_list, _arg3);                                       \
        va_end(_list);                                                         \
        return _name(_1, _2, _3);                                              \
    }                                                                          \
    _ATTR_SECTION(".syscalls")                                                 \
    _ATTR_USED SyscallEntry _g_##_name##_entry =                               \
        (SyscallEntry){.func = _g_##_name##_stub};

#endif // _KERNEL

#endif // !_DXGMX_SYSCALL_TYPES_H
