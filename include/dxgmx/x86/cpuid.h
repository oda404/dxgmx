/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_CPUID_H
#define _DXGMX_X86_CPUID_H

#include <dxgmx/types.h>

#define CPUID(func, eax, ebx, ecx, edx)                                        \
    __asm__ volatile("cpuid"                                                   \
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)              \
                     : "a"(func));

bool cpuid_is_avail();

#endif //_DXGMX_X86_CPUID_H
