/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_CPUID_H
#define _DXGMX_X86_CPUID_H

#include<stdint.h>

#define CPUID(func, eax, ebx, ecx, edx) asm volatile("cpuid": "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx): "a"(func));

int cpuid_is_avail();
void cpuid_get_vendor_str(char *str);
uint32_t cpuid_get_eax_max();

#endif //_DXGMX_X86_CPUID_H
