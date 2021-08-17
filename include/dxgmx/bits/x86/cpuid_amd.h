
#ifndef _DXGMX_BITS_X86_CPUID_AMD_H
#define _DXGMX_BITS_X86_CPUID_AMD_H

#include<dxgmx/types.h>

typedef struct 
S_CPUIDAMDFunction1EAX
{
    u8 stepping:    4;
    u8 base_model:  4;
    u8 base_family: 4;
    u8 reserved:    4;
    u8 ext_model:   4;
    u8 ext_family;
    u8 reserved1:   4;
} CPUIDAMDFunction1EAX;

#endif //!_DXGMX_BITS_X86_CPUIDAMD_H
