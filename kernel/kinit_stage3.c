/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

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
 *
 * Notes:
 * - Do not implement any other functions in here, with or without
 * _ATTR_SECTION(".kinit_stage3_text").
 *
 * - Careful when defining variables that normally end up in the data section.
 * Those will not be accessible from this code, but they can be passed as
 * arguments to syscalls. That's because those variables end up in the kernel
 * data section and therefore are only accessible to kernel code.
 *
 * - Since this code runs in ring 3, obviously do not call kernel code directly,
 * which brings us to the last point.
 *
 * - Careful of writing code that can be inlined using built-in functions, ex:
 * the line "char a [20] = { 0 };" will get translated into assembly using
 * a memset call, even though we use -fno-builtin (maybe I'm missing something).
 */
_ATTR_SECTION(".kinit_stage3_text") _ATTR_NORETURN int kinit_stage3_main()
{
    syscall_ret_t ret = -1;

    /* execve("/bin/main", NULL, NULL) syscall */
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_EXECVE), "b"("/bin/main"), "c"(NULL), "d"(NULL)
        : "memory", "cc", "esi", "edi");

    /* exit(ret) syscall */
    __asm__ volatile("int $0x80"
                     :
                     : "a"(SYSCALL_EXIT), "b"(ret)
                     : "memory", "cc", "ecx", "edx", "esi", "edi");

    /* If exit somehow fails force a null dereference */
    *((volatile int*)NULL) = 0;

    /* If THAT fails just loop */
    while (1)
        ;
}
