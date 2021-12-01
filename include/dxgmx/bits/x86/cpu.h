
#ifndef _DXGMX_BITS_X86_CPU_H
#define _DXGMX_BITS_X86_CPU_H

#include<dxgmx/types.h>

typedef enum E_CR0Flags
{
    /* [Protected mode Enabled] Protected mode is enabled, Else the sys is in real mode. */
    CR0_PE = (1 << 0),
    /* [Monitor co-Processor] idk */
    CR0_MP = (1 << 1),
    /* [EMulation] No x87 floating point unit is present and needs emulation. Else it's present */
    CR0_EM = (1 << 2),
    /* [Task Switched] idk */
    CR0_TS = (1 << 3),
    /* [Extension Type] (i386 specific) External math coprocessor  80287 or 80387 */
    CR0_ET = (1 << 4),
    /* [Numeric Error] x87 internal floating point error reporting. Else PC like x87 error detection is enabled. */
    CR0_NE = (1 << 5),
    /* [Write Protect] Read only pages are enforced in ring 0. Else you can get away with writing to read only pages in ring 0 without a page fault. */
    CR0_WP = (1 << 16),
    /* [Alignment Mask] idk */
    CR0_AM = (1 << 18),
    /* [Not Write-trough] Globall write-through caching is disabled */
    CR0_NW = (1 << 29),
    /* [Cache Disabled] Globally mem cache is disabled. */
    CR0_CD = (1 << 30),
    /* [Paging] Paging enabled. */
    CR0_PG = (1 << 31)
} CR0Flags;

typedef enum E_CR4Flags
{
    /* Virtual 8086 mode extensions. */
    CR4_VME        = 1,
    /* Protected mode virtual interrupts. */
    CR4_PVI        = (1 << 1),
    /* Time stamp disabled. */
    CR4_TSD        = (1 << 2),
    /* Debugging extensions. */
    CR4_DE         = (1 << 3),
    /* Page size extension. */
    CR4_PSE        = (1 << 4),
    /* Page address extension. */
    CR4_PAE        = (1 << 5),
    /* Machine check exception. */
    CR4_MCE        = (1 << 6),
    /* Page global enable */
    CR4_PGE        = (1 << 7),
    /* Performance monitoring counter enable. */
    CR4_PCE        = (1 << 8),
    /* OS support for FXSAVE and FXRSTOR instructions. */
    CR4_OSFXSR     = (1 << 9),
    /* OS support for unmasked SIMD floating point exceptions. */
    CR4_OSXMMEXCPT = (1 << 10),
    /* User mode instruction prevention. General protection fault when 
    executing SGDT, SIDT, SLDT, SMSW, and STR when CPL > 0. */
    CR4_UMIP       = (1 << 11),
    /* Virtual machine extensions. */
    CR4_VMXE       = (1 << 13),
    /* Safer mode extensions. */
    CR4_SMXE       = (1 << 14),
    /* Enable RDFSBASE, RDGSBASE, WRFSBASE and WRGSBASE instructions. */
    CR4_FSGSBASE   = (1 << 16),
    /* Enable PCID. */
    CR4_PCIDE       = (1 << 17),
    /* Enable XSAVE and processor extended status. */
    CR4_OSXSAVE    = (1 << 18),
    /* Supervisor mode execution protection. */
    CR4_SMEP       = (1 << 20),
    /* Supervisor mode access prevention. */
    CR4_SMAP       = (1 << 21),
    /* Enable protection key. */
    CR4_PKE        = (1 << 22),
    /* Control flow enforcement. */
    CR4_CET        = (1 << 23),
    /* Enable protection keys for supervisor mode pages.  */
    CR4_PKS        = (1 << 24),
} CR4Flags;

typedef enum E_CPUVendor
{
    CPU_VENDOR_INTEL,
    CPU_VENDOR_AMD,
} CPUVendor;

typedef enum E_CPUFeatureFlag
{
    /* On chip floating point unit. */
    CPU_FPU       = (1 << 0),
    /* Virtual mode enhancements */
    CPU_VME       = (1 << 1),
    /* Debugging extension. */
    CPU_DE        = (1 << 2),
    /* Page size extension. */
    CPU_PSE       = (1 << 3),
    /* Time stamp counter. */
    CPU_TSC       = (1 << 4),
    /* Model specifc registers. */
    CPU_MSR       = (1 << 5),
    /* Page address extension/ */
    CPU_PAE       = (1 << 6),
    /* Machine check exception. */
    CPU_MCE       = (1 << 7),
    /* CMPXCHG8B instruction. */
    CPU_CMPXCHG8B = (1 << 8),
    /* APIC is present and enabled. */
    CPU_APIC      = (1 << 9),
    /* SYSENTER and SYSEXIT instructions. */
    CPU_SEP       = (1 << 10),
    /* Memory type range registers. */
    CPU_MTRR      = (1 << 11),
    /* Page global extension. */
    CPU_PGE       = (1 << 12),
    /* Machine check architecture. */
    CPU_MCA       = (1 << 13),
    /* CMOV instruction. */
    CPU_CMOV      = (1 << 14),
    /* Page attribute table. */
    CPU_PAT       = (1 << 15),
    /* PSE32 page size extension. */
    CPU_PSE36     = (1 << 16),
    /* CLFLUSH instruction. */
    CPU_CLFLUSH   = (1 << 17),
    /* MMX instruction/ */
    CPU_MMX       = (1 << 18),
    /* FXSR and FXSAVE instructions. */
    CPU_FXSR      = (1 << 19),
    /* SSE instruction. */
    CPU_SSE       = (1 << 20),
    /* SSE2 instruction. */
    CPU_SSE2      = (1 << 21),
    /* Hyper threading technology. */
    CPU_HTT       = (1 << 22),
    /* Processor serial number present and enabled. */
    CPU_PSN       = (1 << 23),
    /* Debug store. */
    CPU_DS        = (1 << 24),
    /* Thermal monitor and sofware controlled clock facilities. */
    CPU_ACPI      = (1 << 25),
    /* Self snoop. */
    CPU_SS        = (1 << 26),
    /* Automatic thermal control circuit (TCC). */
    CPU_TM        = (1 << 27),

    // ... more to come
} CPUFeatureFlag;

typedef enum E_EFERFlags
{
    EFER_SCE = 1,
    EFER_LME = (1 << 8),
    EFER_LMA = (1 << 10),
    EFER_NXE = (1 << 11),
    EFER_SVME = (1 << 12),
    EFER_LMSLE = (1 << 13),
    EFER_FFXSR = (1 << 14),
    EFER_TCE = (1 << 15)
} EFERFlags;

typedef enum E_CPUMSR
{
    MSR_EFER = 0xC0000080
} CPUMSR;

typedef struct S_CPUFeatures
{
    union
    {
        struct _ATTR_PACKED
        {
            u8 fpu: 1;
            u8 vme: 1;
            u8 de: 1;
            u8 pse: 1;
            u8 tsc: 1;
            u8 msr: 1;
            u8 pae: 1;
            u8 mce: 1;
            u8 cmpxchg8: 1;
            u8 apic: 1;
            u8 sep: 1;
            u8 mtrr: 1;
            u8 pge: 1;
            u8 mca: 1;
            u8 cmov: 1;
            u8 pat: 1;
            u8 pse36: 1;
            u8 clflush: 1;
            u8 mmx: 1;
            u8 fxsr: 1;
            u8 sse: 1;
            u8 sse2: 1;
            u8 htt: 1;
            u8 psn: 1;
            u8 ds: 1;
            u8 acpi: 1;
            u8 ss: 1;
            u8 tm: 1;
        };
        u64 flags;
    };
} CPUFeatures;

u32 cpu_read_cr0();
u32 cpu_read_cr2();
u32 cpu_read_cr3();
u32 cpu_read_cr4();

u32 cpu_read_ebp();
u32 cpu_read_esp();

void cpu_write_cr0(u32 val);
void cpu_write_cr4(u32 val);
void cpu_write_cr3(u32 val);

u64 cpu_read_msr(CPUMSR msr);
void cpu_write_msr(u64 val, CPUMSR msr);

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
