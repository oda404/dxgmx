/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/panic.h>
#include <dxgmx/types.h>

/* TODO: actually randomize it */
ptr __stack_chk_guard = 0xFA7BA115;

_ATTR_NORETURN void __stack_chk_fail()
{
    panic("Stack smashing detected in ring 0 :(");
}
