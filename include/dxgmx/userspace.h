/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_USERSPACE_H
#define _DXGMX_USERSPACE_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

_ATTR_NORETURN void userspace_jump2user(ptr instrptr, ptr stackptr);
_ATTR_NORETURN void userspace_arch_jump2user(ptr instrptr, ptr stackptr);

#endif // !_DXGMX_USERSPACE_H
