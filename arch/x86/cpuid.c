/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/generated/kconfig.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/cpuid.h>

bool cpuid_is_avail()
{
    bool ret;
#ifdef CONFIG_64BIT
    __asm__ volatile(
        "pushfq                                                                                   \n"
        "pushfq                                                                                   \n"
        "xorq   $0x200000, (%%rsp)  # flip bit 21                                                 \n"
        "popfq                      # pop modified eflags                                         \n"
        "pushfq                     # push 'modified' eflags                                      \n"
        "popq   %%rax                                                                             \n"
        "xor    (%%rsp),   %%rax    # xor 'modified' and original together to see if they changed \n"
        "and    $0x200000, %%rax                                                                  \n"
        "shr    $21,       %%rax                                                                  \n"
        "popfq                      # restore original eflags                                     \n"
        : "=a"(ret));
#else
    __asm__ volatile(
        "pushfl                                                                                   \n"
        "pushfl                                                                                   \n"
        "xorl   $0x200000, (%%esp)  # flip bit 21                                                 \n"
        "popfl                      # pop modified eflags                                         \n"
        "pushfl                     # push 'modified' eflags                                      \n"
        "popl   %%eax                                                                             \n"
        "xor    (%%esp),   %%eax    # xor 'modified' and original together to see if they changed \n"
        "and    $0x200000, %%eax                                                                  \n"
        "shr    $21,       %%eax                                                                  \n"
        "popfl                      # restore original eflags                                     \n"
        : "=a"(ret));
#endif // CONFIG_64BIT
    return ret;
}
