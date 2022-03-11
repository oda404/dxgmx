/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/kstack.h>
#include <dxgmx/types.h>

_ATTR_ALIGNED(4096) _ATTR_SECTION(".bss") static u8 g_kstack[KSTACK_SIZE] = {0};

ptr _kernel_stack_top = (ptr)&g_kstack[KSTACK_SIZE - 1];
ptr _kernel_stack_bot = (ptr)&g_kstack[0];
