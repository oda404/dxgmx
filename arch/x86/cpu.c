/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/x86/cmos.h>
#include <dxgmx/x86/cpuid.h>
#include <dxgmx/x86/interrupts.h>

static CPUFeatures g_cpufeatures;
static CPUInfo g_cpuinfo;
static int g_cpu_identified = 0;

#define CPU_VENDORSTR_INTEL "GenuineIntel"
#define CPU_VENDORSTR_AMD "AuthenticAMD"

#define KLOGF_PREFIX "cpu: "

_INIT static void cpu_set_common_features(u32 edx)
{
    g_cpufeatures.fpu = edx & 1;
    g_cpufeatures.vme = (edx >> 1) & 1;
    g_cpufeatures.de = (edx >> 2) & 1;
    g_cpufeatures.pse = (edx >> 3) & 1;
    g_cpufeatures.tsc = (edx >> 4) & 1;
    g_cpufeatures.msr = (edx >> 5) & 1;
    g_cpufeatures.pae = (edx >> 6) & 1;
    g_cpufeatures.mce = (edx >> 7) & 1;
    g_cpufeatures.cmpxchg8 = (edx >> 8) & 1;
    g_cpufeatures.apic = (edx >> 9) & 1;
    g_cpufeatures.sep = (edx >> 11) & 1;
    g_cpufeatures.mtrr = (edx >> 12) & 1;
    g_cpufeatures.pge = (edx >> 13) & 1;
    g_cpufeatures.mca = (edx >> 14) & 1;
    g_cpufeatures.cmov = (edx >> 15) & 1;
    g_cpufeatures.pat = (edx >> 16) & 1;
    g_cpufeatures.pse36 = (edx >> 17) & 1;
    g_cpufeatures.clflush = (edx >> 19) & 1;
    g_cpufeatures.mmx = (edx >> 23) & 1;
    g_cpufeatures.fxsr = (edx >> 24) & 1;
    g_cpufeatures.sse = (edx >> 25) & 1;
    g_cpufeatures.sse2 = (edx >> 26) & 1;
    g_cpufeatures.htt = (edx >> 28) & 1;
}

_INIT static void cpu_handle_amd_cpuid()
{
    u32 eax, ebx, ecx, edx;
    CPUID(1, eax, ebx, ecx, edx);

    g_cpuinfo.vendor = CPU_VENDOR_AMD;

#include <dxgmx/bits/x86/cpuid_amd.h>

    // https://www.amd.com/system/files/TechDocs/25481.pdf

    CPUIDAMDFunction1EAX f1eax = *(CPUIDAMDFunction1EAX*)&eax;

    g_cpuinfo.stepping = f1eax.stepping;

    if (f1eax.base_family < 0xF)
    {
        g_cpuinfo.family = f1eax.base_family;
        g_cpuinfo.model = f1eax.base_model;
    }
    else
    {
        g_cpuinfo.family = f1eax.base_family + f1eax.ext_family;
        g_cpuinfo.model = f1eax.ext_model * 0x10 + f1eax.base_model;
    }

    cpu_set_common_features(edx);
}

_INIT static void cpu_handle_intel_cpuid()
{
    u32 eax, ebx, ecx, edx;
    g_cpuinfo.vendor = CPU_VENDOR_INTEL;

#include <dxgmx/bits/x86/cpuid_intel.h>

    // https://www.microbe.cz/docs/CPUID.pdf

    CPUID(1, eax, ebx, ecx, edx);
    IntelCPUIDFunc1EDX f1eax = *(IntelCPUIDFunc1EDX*)&eax;

    g_cpuinfo.family = f1eax.extended_family + f1eax.family_code;
    g_cpuinfo.model = (f1eax.extended_model << 4) + f1eax.model_number;
    g_cpuinfo.stepping = f1eax.stepping_id;

    cpu_set_common_features(edx);
    /* Intel specific features. */
    g_cpufeatures.psn = (edx >> 18) & 1;
    g_cpufeatures.ds = (edx >> 21) & 1;
    g_cpufeatures.acpi = (edx >> 22) & 1;
    g_cpufeatures.ss = (edx >> 27) & 1;
    g_cpufeatures.tm = (edx >> 29) & 1;
}

_INIT int cpu_identify()
{
    if (!cpuid_is_avail())
    {
        panic("CPUID is not available... tf are you running this thing on?");
        return 1;
    }

    CPUID(
        0,
        g_cpuinfo.cpuid_eaxmax,
        *(u32*)&g_cpuinfo.vendorstr[0],
        *(u32*)&g_cpuinfo.vendorstr[8],
        *(u32*)&g_cpuinfo.vendorstr[4]);

    KLOGF(INFO, "Vendor string is \"%s\".", g_cpuinfo.vendorstr);

    memset(&g_cpufeatures, 0, sizeof(g_cpufeatures));

    if (strcmp(g_cpuinfo.vendorstr, CPU_VENDORSTR_AMD) == 0)
        cpu_handle_amd_cpuid();
    else if (strcmp(g_cpuinfo.vendorstr, CPU_VENDORSTR_INTEL) == 0)
        cpu_handle_intel_cpuid();
    else
        panic("Unknown CPU vendor '%s'. Not proceeding.", g_cpuinfo.vendorstr);

    KLOGF(
        INFO,
        "UID is %d.%d.%d.",
        g_cpuinfo.family,
        g_cpuinfo.model,
        g_cpuinfo.stepping);
    g_cpu_identified = 1;

    return 0;
}

const CPUInfo* cpu_get_info()
{
    if (!g_cpu_identified)
        return NULL;

    return &g_cpuinfo;
}

bool cpu_has_feature(CPUFeatureFlag flag)
{
    return (bool)(g_cpufeatures.flags & flag);
}

u32 cpu_read_cr2()
{
    u32 ret;
    __asm__ volatile("movl %%cr2, %0" : "=a"(ret));
    return ret;
}

u32 cpu_read_cr3()
{
    u32 ret;
    __asm__ volatile("movl %%cr3, %0" : "=a"(ret));
    return ret;
}

u32 cpu_read_cr0()
{
    u32 ret;
    __asm__ volatile("movl %%cr0, %0" : "=a"(ret));
    return ret;
}

u32 cpu_read_ebp()
{
    u32 ret;
    __asm__ volatile("movl %%ebp, %0" : "=a"(ret));
    return ret;
}

u32 cpu_read_esp()
{
    u32 ret;
    __asm__ volatile("movl %%esp, %0" : "=a"(ret));
    return ret;
}

u32 cpu_read_cr4()
{
    u32 ret;
    __asm__ volatile("movl %%cr4, %0" : "=a"(ret));
    return ret;
}

u64 cpu_read_msr(CPUMSR msr)
{
    if (!cpu_has_feature(CPU_MSR))
        return 0;
    u32 hi, lo;
    __asm__ volatile("rdmsr" : "=d"(hi), "=a"(lo) : "c"((u32)msr));
    return ((u64)hi << 32) | lo;
}

_INIT void cpu_write_cr0(u32 val)
{
    __asm__ volatile("movl %0, %%cr0" : : "a"(val));
}

_INIT void cpu_write_cr3(u32 val)
{
    __asm__ volatile("movl %0, %%cr3" : : "a"(val));
}

_INIT void cpu_write_cr4(u32 val)
{
    __asm__ volatile("movl %0, %%cr4" : : "a"(val));
}

_INIT void cpu_write_msr(u64 val, CPUMSR msr)
{
    if (!cpu_has_feature(CPU_MSR))
        return;
    __asm__ volatile("wrmsr"
                     :
                     : "d"((u32)(val >> 32)), "a"((u32)val), "c"((u32)msr));
}

void cpu_suspend()
{
    __asm__ volatile("hlt");
}

void cpu_hang()
{
    while (1)
    {
        interrupts_disable();
        cpu_suspend();
    }
}

void cpu_enable_irqs()
{
    __asm__ volatile("cli");
}

void cpu_disable_irqs()
{
    __asm__ volatile("sti");
}
