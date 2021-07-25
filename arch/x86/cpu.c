
#include<dxgmx/cpu.h>
#include<stdint.h>
#include<dxgmx/x86/cpuid.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/string.h>

static char vendor_str[12];
static uint32_t eax_max = -1;

#define CPU_AMD   0x0
#define CPU_INTEL 0x1

#define CPU_VENDOR_INTEL_STR "GenuineIntel"
#define CPU_VENDOR_AMD_STR   "AuthenticAMD"

int cpu_identify()
{
    if(!cpuid_is_avail())
    {
        abandon_ship("CPUID is not available... tf are you running this thing on?\n");
        return 1;
    }

    cpuid_get_vendor_str(vendor_str);
    eax_max = cpuid_get_eax_max();

    kprintf("CPU vendor: %s\n", vendor_str);

    if(strcmp(vendor_str, CPU_VENDOR_AMD_STR) == 0)
    {
        // TODO
    }
    else if(strcmp(vendor_str, CPU_VENDOR_INTEL_STR) == 0)
    {
        // TODO
    }
    else
    {
        abandon_ship("Unsupported CPU vendor '%s'. Not proceeding.\n", vendor_str);
        return 2;
    }

    return 0;
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
