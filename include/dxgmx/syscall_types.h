/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_SYSCALL_TYPES_H
#define _DXGMX_SYSCALL_TYPES_H

#include <dxgmx/posix/sys/types.h>

typedef size_t syscall_t;
typedef size_t syscall_arg_t;
typedef ssize_t syscall_ret_t;

#ifdef _KERNEL
#include <stdarg.h>

typedef struct SyscallEntry
{
    syscall_ret_t (*func)(va_list*);
} SyscallEntry;

#define SYSCALL_VOID_1(_name, _arg1)                                           \
    void _name(_arg1);                                                         \
    static syscall_ret_t _g_##_name##_stub(va_list* _list)                     \
    {                                                                          \
        _arg1 _1 = va_arg(*_list, _arg1);                                      \
        va_end(*_list);                                                        \
        _name(_1);                                                             \
        return 0;                                                              \
    }                                                                          \
    _ATTR_SECTION(".syscalls")                                                 \
    _ATTR_USED SyscallEntry _g_##_name##_entry =                               \
        (SyscallEntry){.func = _g_##_name##_stub};

#define SYSCALL_RETV_3(_ret, _name, _arg1, _arg2, _arg3)                       \
    _ret _name(_arg1, _arg2, _arg3);                                           \
    static syscall_ret_t _g_##_name##_stub(va_list* _list)                     \
    {                                                                          \
        _arg1 _1 = va_arg(*_list, _arg1);                                      \
        _arg2 _2 = va_arg(*_list, _arg2);                                      \
        _arg3 _3 = va_arg(*_list, _arg3);                                      \
        va_end(*_list);                                                        \
        return _name(_1, _2, _3);                                              \
    }                                                                          \
    _ATTR_SECTION(".syscalls")                                                 \
    _ATTR_USED SyscallEntry _g_##_name##_entry =                               \
        (SyscallEntry){.func = _g_##_name##_stub};

#define SYSCALL_RETV_6(_ret, _name, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6)  \
    _ret _name(_arg1, _arg2, _arg3, _arg4, _arg5, _arg6);                      \
    static syscall_ret_t _g_##_name##_stub(va_list* _list)                     \
    {                                                                          \
        _arg1 _1 = va_arg(*_list, _arg1);                                      \
        _arg2 _2 = va_arg(*_list, _arg2);                                      \
        _arg3 _3 = va_arg(*_list, _arg3);                                      \
        _arg4 _4 = va_arg(*_list, _arg4);                                      \
        _arg5 _5 = va_arg(*_list, _arg5);                                      \
        _arg6 _6 = va_arg(*_list, _arg6);                                      \
        va_end(*_list);                                                        \
        return (syscall_ret_t)_name(_1, _2, _3, _4, _5, _6);                   \
    }                                                                          \
    _ATTR_SECTION(".syscalls")                                                 \
    _ATTR_USED SyscallEntry _g_##_name##_entry =                               \
        (SyscallEntry){.func = _g_##_name##_stub};

#define SYSCALL_NULL(_n)                                                       \
    int sys_undefined(syscall_t);                                              \
    static syscall_ret_t _g_null_##_n##_stub(va_list* _list)                   \
    {                                                                          \
        va_end(*_list);                                                        \
        return sys_undefined(_n);                                              \
    }                                                                          \
    _ATTR_SECTION(".syscalls")                                                 \
    _ATTR_USED SyscallEntry _g_null_##_n##_entry =                             \
        (SyscallEntry){.func = _g_null_##_n##_stub};

#endif // _KERNEL

#endif // !_DXGMX_SYSCALL_TYPES_H
