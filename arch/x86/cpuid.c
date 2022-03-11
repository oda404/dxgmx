/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/string.h>
#include <dxgmx/x86/cpuid.h>

bool cpuid_is_avail()
{
    bool ret;
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
    return ret;
}
