/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/syscalls.h>

/**
 * kinit_stage3 is a userspace binary image baked into the kernel.
 * I remember reading somewhere that linux does something similar, and it
 * seems like a really elegant solution on how to spawn the init process without
 * the kernel reading the binary off of disk using non-syscalls methods.
 * This way we spawn this binary as pid 1 and then, let it spawn the actual
 * pid 1 using the exec syscall. If you're curious about how this image is
 * loaded in memory check procm_spawn_init().
 */

_ATTR_SECTION(".kinit_stage3_text") _ATTR_NORETURN int kinit_stage3_main()
{

    /* exit(0) syscall */
    __asm__ volatile("int $0x80"
                     :
                     : "a"(SYSCALL_EXIT), "b"(0)
                     : "memory", "cc", "ecx", "edx", "esi", "edi");

    ASSERT_NOT_HIT();
}
