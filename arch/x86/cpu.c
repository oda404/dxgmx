/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/cpu.h>
#include<dxgmx/x86/cpuid.h>
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<dxgmx/string.h>
#include<dxgmx/todo.h>
#include<dxgmx/attrs.h>

static CPUInfo g_cpuinfo;
static int g_cpu_identified = 0;

#define CPU_VENDORSTR_INTEL "GenuineIntel"
#define CPU_VENDORSTR_AMD   "AuthenticAMD"

#define KLOGF(lvl, fmt, ...) klogln(lvl, "cpu: " fmt, ##__VA_ARGS__);

_INIT static void cpu_handle_amd_cpuid()
{
    uint32_t eax, ebx, ecx, edx;
    CPUID(1, eax, ebx, ecx, edx);

    g_cpuinfo.vendor = CPU_VENDOR_AMD;

#include<dxgmx/bits/x86/cpuid_amd.h>

    //https://www.amd.com/system/files/TechDocs/25481.pdf

    CPUIDAMDFunction1EAX f1eax = *(CPUIDAMDFunction1EAX*)&eax;

    g_cpuinfo.stepping = f1eax.stepping;

    if(f1eax.base_family < 0xF)
    {
        g_cpuinfo.family = f1eax.base_family;
        g_cpuinfo.model = f1eax.base_model;
    }
    else
    {
        g_cpuinfo.family = f1eax.base_family + f1eax.ext_family;
        g_cpuinfo.model = f1eax.ext_model * 0x10 + f1eax.base_model;
    }

    KLOGF(INFO, "UID is %d.%d.%d.", g_cpuinfo.family, g_cpuinfo.model, g_cpuinfo.stepping);
}

_INIT static void cpu_handle_intel_cpuid()
{
    u32 eax, ebx, ecx, edx;
    g_cpuinfo.vendor = CPU_VENDOR_INTEL;

#include<dxgmx/bits/x86/cpuid_intel.h>

    //https://www.microbe.cz/docs/CPUID.pdf

    CPUID(1, eax, ebx, ecx, edx);
    IntelCPUIDFunc1EDX f1eax = *(IntelCPUIDFunc1EDX*)&eax;

    g_cpuinfo.family = f1eax.extended_family + f1eax.family_code;
    g_cpuinfo.model = (f1eax.extended_model << 4) + f1eax.model_number;
    g_cpuinfo.stepping = f1eax.stepping_id;

    KLOGF(INFO, "UID is %d.%d.%d.", g_cpuinfo.family, g_cpuinfo.model, g_cpuinfo.stepping);
}

_INIT int cpu_identify()
{
    if(!cpuid_is_avail())
    {
        abandon_ship("CPUID is not available... tf are you running this thing on?");
        return 1;
    }

    CPUID(
        0, 
        g_cpuinfo.cpuid_eaxmax, 
        *(u32*)&g_cpuinfo.vendorstr[0], 
        *(u32*)&g_cpuinfo.vendorstr[8], 
        *(u32*)&g_cpuinfo.vendorstr[4]
    );

    KLOGF(INFO, "Vendor string is \"%s\".", g_cpuinfo.vendorstr);

    if(strcmp(g_cpuinfo.vendorstr, CPU_VENDORSTR_AMD) == 0)
        cpu_handle_amd_cpuid();
    else if(strcmp(g_cpuinfo.vendorstr, CPU_VENDORSTR_INTEL) == 0)
        cpu_handle_intel_cpuid();
    else
        abandon_ship("Unknown CPU vendor '%s'. Not proceeding.", g_cpuinfo.vendorstr);

    g_cpu_identified = 1;

    return 0;
}

const CPUInfo *cpu_get_info()
{
    if(!g_cpu_identified)
        return NULL;
        
    return &g_cpuinfo;
}

uint32_t cpu_read_cr2()
{
    uint32_t ret;
    __asm__ volatile("movl %%cr2, %0": "=a"(ret));
    return ret;
}

uint32_t cpu_read_cr0()
{
    uint32_t ret;
    __asm__ volatile("movl %%cr0, %0": "=a"(ret));
    return ret;
}

u32 cpu_read_cr4()
{
    u32 ret;
    __asm__ volatile("movl %%cr4, %0": "=a"(ret));
    return ret;
}

_INIT void cpu_set_cr0(uint32_t val)
{
    __asm__ volatile("movl %0, %%cr0": :"a"(val));
}

_INIT void cpu_set_cr4(u32 val)
{
    __asm__ volatile("movl %0, %%cr4": : "a"(val));
}

void cpu_suspend()
{
    __asm__ volatile("hlt");
}

void cpu_hang()
{
    while(1)
    {
        /** 
         * CPU should remain suspended after the first iteration...
         * but I'm paranoid.
        */
        cmos_disable_nmi();
        interrupts_disable();

        cpu_suspend();
    }
    __builtin_unreachable();
}
