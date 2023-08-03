/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_USERSPACE_H
#define _DXGMX_USERSPACE_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

_CDECL _ATTR_NEVER_INLINE int user_copy_to(void* dest, void* src, size_t n);
int user_access_fault_stub();

_ATTR_NORETURN void user_jump2user(ptr instrptr, ptr stackptr);

#endif // !_DXGMX_USERSPACE_H
