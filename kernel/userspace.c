/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/userspace.h>

void userspace_jump2user(ptr instrptr, ptr stackptr)
{
    userspace_arch_jump2user(instrptr, stackptr);
}
