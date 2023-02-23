/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/cpu.h>
#include <dxgmx/klog.h>
#include <dxgmx/x86/cmos.h>
#include <dxgmx/x86/cpuid.h>
#include <dxgmx/x86/pic.h>
#include <dxgmx/x86/tsc.h>

static u64 g_tsc_freq = 0;

int tsc_init()
{
    u32 cr4 = cpu_read_cr4();
    /* Bit 2 of the cr4 registers signals whether
    the TSC is enabled or not. */
    if (!(cr4 & (1 << 2)))
        cpu_set_cr4(cr4 | (1 << 2));

    return 0;
}

int tsc_is_available()
{
    u32 edx, dead;
    CPUID(1, dead, dead, dead, edx);
    return (edx >> 4) & 1;
}

int tsc_is_constant()
{
    const CPUInfo* cpuinfo = cpu_get_info();
    ASSERT(cpuinfo);
    /* the following bit is stolen straight from the linux source. */
    switch (cpuinfo->vendor)
    {
    case CPU_VENDOR_INTEL:
        if ((cpuinfo->family == 0xF && cpuinfo->model >= 3) ||
            (cpuinfo->family == 6 && cpuinfo->model >= 0xE))
            return 1;
        break;

    case CPU_VENDOR_AMD:
        return tsc_is_invariant();

    default:
        /* Shouldn't really happen, as stuff like this should be
        caught by cpu_identify. */
        panic("Unknown cpu vendor id: %u. Not proceeding.", cpuinfo->vendor);
        return 0;
    }

    return 0;
}

int tsc_is_invariant()
{
    u32 edx, dead;
    CPUID(0x80000007, dead, dead, dead, edx);
    return edx & (1 << 8);
}

__attribute__((noinline)) u64 tsc_read()
{
    u32 edx, eax;
    asm volatile("RDTSC" : "=d"(edx), "=a"(eax));
    return ((u64)edx << 32) | eax;
}
