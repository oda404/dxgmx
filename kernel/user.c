/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/user.h>

void user_jump2user(ptr instrptr, ptr stackptr)
{
    extern _ATTR_NORETURN void user_jump2user_arch(ptr instrptr, ptr stackptr);
    user_jump2user_arch(instrptr, stackptr);
}
