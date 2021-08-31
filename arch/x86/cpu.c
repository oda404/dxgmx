/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/cpu.h>
#include<stdint.h>
#include<dxgmx/x86/cpuid.h>
#include<dxgmx/x86/cmos.h>
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<dxgmx/string.h>
#include<dxgmx/todo.h>

static CPUInfo g_cpuinfo;

#define CPU_AMD   0x0
#define CPU_INTEL 0x1

#define CPU_VENDORSTR_INTEL "GenuineIntel"
#define CPU_VENDORSTR_AMD   "AuthenticAMD"

#define KLOGF(lvl, fmt, ...) klog(lvl, "[CPU] " fmt, ##__VA_ARGS__);

static void cpu_handle_amd_cpuid()
{
    uint32_t eax, ebx, ecx, edx;
    CPUID(1, eax, ebx, ecx, edx);

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

    KLOGF(KLOG_INFO, "UID: %d.%d.%d\n", g_cpuinfo.family, g_cpuinfo.model, g_cpuinfo.stepping);
}

static void cpu_handle_intel_cpuid()
{
    TODO_FATAL();
}

int cpu_identify()
{
    if(!cpuid_is_avail())
    {
        abandon_ship("CPUID is not available... tf are you running this thing on?\n");
        return 1;
    }

    CPUID(
        0, 
        g_cpuinfo.cpuid_eaxmax, 
        *(u32*)&g_cpuinfo.vendorstr[0], 
        *(u32*)&g_cpuinfo.vendorstr[8], 
        *(u32*)&g_cpuinfo.vendorstr[4]
    );

    KLOGF(KLOG_INFO, "Vendor: %s\n", g_cpuinfo.vendorstr);

    if(strcmp(g_cpuinfo.vendorstr, CPU_VENDORSTR_AMD) == 0)
        cpu_handle_amd_cpuid();
    else if(strcmp(g_cpuinfo.vendorstr, CPU_VENDORSTR_INTEL) == 0)
        cpu_handle_intel_cpuid();
    else
        abandon_ship("Unknown CPU vendor '%s'. Not proceeding.\n", g_cpuinfo.vendorstr);

    return 0;
}

const CPUInfo *cpu_get_info()
{
    return &g_cpuinfo;   
}

uint32_t cpu_get_cr2()
{
    uint32_t ret;
    asm volatile("movl %%cr2, %0": "=a"(ret));
    return ret;
}

uint32_t cpu_get_cr0()
{
    uint32_t ret;
    asm volatile("movl %%cr0, %0": "=a"(ret));
    return ret;
}

void cpu_set_cr0(uint32_t val)
{
    asm volatile("movl %0, %%cr0": :"a"(val));
}

void cpu_suspend()
{
    asm volatile("hlt");
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
