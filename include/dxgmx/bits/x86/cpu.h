
#ifndef _DXGMX_BITS_X86_CPU_H
#define _DXGMX_BITS_X86_CPU_H

#include<dxgmx/types.h>

typedef enum E_CR0Flags
{
    /* [Protected mode Enabled] Protected mode is enabled, Else the sys is in real mode. */
    CR0FLAG_PE = (1 << 0),
    /* [Monitor co-Processor] idk */
    CR0FLAG_MP = (1 << 1),
    /* [EMulation] No x87 floating point unit is present and needs emulation. Else it's present */
    CR0FLAG_EM = (1 << 2),
    /* [Task Switched] idk */
    CR0FLAG_TS = (1 << 3),
    /* [Extension Type] (i386 specific) External math coprocessor  80287 or 80387 */
    CR0FLAG_ET = (1 << 4),
    /* [Numeric Error] x87 internal floating point error reporting. Else PC like x87 error detection is enabled. */
    CR0FLAG_NE = (1 << 5),
    /* [Write Protect] Read only pages are enforced in ring 0. Else you can get away with writing to read only pages in ring 0 without a page fault. */
    CR0FLAG_WP = (1 << 16),
    /* [Alignment Mask] idk */
    CR0FLAG_AM = (1 << 18),
    /* [Not Write-trough] Globall write-through caching is disabled */
    CR0FLAG_NW = (1 << 29),
    /* [Cache Disabled] Globally mem cache is disabled. */
    CR0FLAG_CD = (1 << 30),
    /* [Paging] Paging enabled. */
    CR0FLAG_PG = (1 << 31)
} CR0Flags;

typedef enum E_CPUVendor
{
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
} CPUVendor;

u32 cpu_read_cr2();
u32 cpu_read_cr0();
u32 cpu_read_cr4();
void cpu_set_cr0(u32 val);
void cpu_set_cr4(u32 val);

typedef struct
S_CPUInfo
{
    u8   vendor;
    char vendorstr[13];
    u32  cpuid_eaxmax;
    u8   stepping;
    u8   model;
    u8   family;
    u8   local_apic_id;
} CPUInfo;

#endif //!_DXGMX_BITS_X86_CPU_H