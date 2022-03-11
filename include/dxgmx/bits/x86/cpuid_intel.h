/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_BITS_X86_CPUID_INTEL_H
#define _DXGMX_BITS_X86_CPUID_INTEL_H

#include <dxgmx/types.h>

typedef struct S_IntelCPUIDFunc1EDX
{
    u8 stepping_id : 4;
    u8 model_number : 4;
    u8 family_code : 4;
    u8 type : 2;
    u8 reserved : 2;
    u8 extended_model : 4;
    u8 extended_family;
    u8 reserved1 : 4;
} IntelCPUIDFunc1EDX;

#endif //!_DXGMX_BITS_X86_CPUID_INTEL_H
