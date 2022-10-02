/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/acpi.h>
#include <dxgmx/x86/mm.h>

#define KLOGF_PREFIX "acpi: "

extern u8 _kernel_map_offset[];
static const AcpiRsdTable* g_rsd_table = NULL;
static volatile AcpiHpetTable* g_hpet_table = NULL;

static _INIT AcpiRsdPointer* acpi_find_rsdp()
{
    u32 ebda = (*(u16*)(0x40E + (ptr)_kernel_map_offset)) << 4;
    ebda += (ptr)_kernel_map_offset;

    for (ptr i = ebda; i < ebda + 0x400; i += 16)
    {
        if (memcmp((void*)i, "RSD PTR ", 8) == 0)
            return (AcpiRsdPointer*)i;
    }

    for (ptr i = 0xE0000 + (ptr)_kernel_map_offset;
         i < 0x100000 + (ptr)_kernel_map_offset;
         i += 16)
    {
        if (memcmp((void*)i, "RSD PTR ", 8) == 0)
            return (AcpiRsdPointer*)i;
    }

    return NULL;
}

static _INIT bool acpi_is_rsdp_valid(const AcpiRsdPointer* rsdp)
{
    if (!rsdp)
        return false;

    u32 sum = 0;
    for (size_t i = 0; i < sizeof(AcpiRsdPointerV1); ++i)
        sum += *((const u8*)rsdp + i);

    if (rsdp->rsdp_v1.rev == 2)
    {
        for (size_t i = sizeof(AcpiRsdPointerV1); i < sizeof(AcpiRsdPointerV2);
             ++i)
            sum += *((const u8*)rsdp + i);
    }

    return !(sum & 0xFF);
}

static _INIT bool acpi_is_sdt_header_valid(const AcpiSdTableHeader* header)
{
    if (!header)
        return false;

    u32 sum = 0;
    for (size_t i = 0; i < header->len; ++i)
        sum += *((const u8*)header + i);

    return !(sum & 0xFF);
}

static int
acpi_reserve_table(const char* name, size_t size, AcpiSdTableHeader* hdr)
{
    const int st = mm_reserve_acpi_range((ptr)hdr, size);

    if (!st)
        KLOGF(INFO, "Reserved %s table at 0x%p.", name, (void*)hdr);
    else
        KLOGF(
            ERR,
            "Failed to reserve %s table at 0x%p, error: %d.",
            name,
            (void*)hdr,
            st);

    return st;
}

_INIT bool acpi_reserve_tables()
{
    AcpiRsdPointer* rsdp = acpi_find_rsdp();

    if (!acpi_is_rsdp_valid(rsdp))
    {
        KLOGF(WARN, "RSDP at 0x%p is invalid!", rsdp);
        return false;
    }

    int st = acpi_reserve_table(
        "RSD",
        sizeof(AcpiRsdTable),
        (AcpiSdTableHeader*)rsdp->rsdp_v1.rsdt_base);

    if (st)
        return st;

    g_rsd_table = (AcpiRsdTable*)rsdp->rsdp_v1.rsdt_base;

    if (!acpi_is_sdt_header_valid(&g_rsd_table->header))
    {
        KLOGF(WARN, "RSDT at 0x%p is invalid!", g_rsd_table);
        return false;
    }

    const size_t rsdt_tables =
        (g_rsd_table->header.len - sizeof(g_rsd_table->header)) / 4;

    AcpiSdTableHeader* header = (void*)(g_rsd_table->tables);
    for (size_t i = 0; i < rsdt_tables; ++i)
    {
        if (acpi_is_sdt_header_valid(header))
        {
            if (memcmp(header->signature, "HPET", 4) == 0)
            {
                st = acpi_reserve_table("HPET", sizeof(AcpiHpetTable), header);
                if (st)
                    continue;

                g_hpet_table = (AcpiHpetTable*)header;
            }
        }
        else
        {
            KLOGF(WARN, "Found invalid header at 0x%p.", header);
        }

        header = (AcpiSdTableHeader*)((u8*)header + header->len);
    }

    return true;
}

volatile AcpiHpetTable* acpi_get_hpet_table()
{
    return g_hpet_table;
}
