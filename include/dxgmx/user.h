/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_USERSPACE_H
#define _DXGMX_USERSPACE_H

#include <dxgmx/attrs.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/err_or.h>
#include <dxgmx/types.h>

_CDECL _ATTR_NEVER_INLINE int
user_copy_to(_USERPTR void* dest, const void* src, size_t n);

_CDECL _ATTR_NEVER_INLINE int
user_copy_str_from(_USERPTR const void* src, void* dest, size_t maxn);

_CDECL _ATTR_NEVER_INLINE int
user_copy_from(const void* _USERPTR src, void* dest, size_t n);

_CDECL _ATTR_NEVER_INLINE ssize_t
user_strnlen(const void* _USERPTR str, size_t n);

ERR_OR_PTR(char) user_strndup(const void* _USERPTR str, size_t maxn);

int user_access_fault_stub();

_ATTR_NORETURN void user_enter_arch(ptr ip, ptr sp);

#endif // !_DXGMX_USERSPACE_H
