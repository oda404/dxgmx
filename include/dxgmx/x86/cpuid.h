/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_CPUID_H
#define _DXGMX_X86_CPUID_H

#define CPUID(func, eax, ebx, ecx, edx) __asm__ volatile("cpuid": "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx): "a"(func));

int cpuid_is_avail();

#endif //_DXGMX_X86_CPUID_H
