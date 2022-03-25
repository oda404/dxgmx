/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/mmanager.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/acpi.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "acpi: " fmt, ##__VA_ARGS__)

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

static _INIT void acpi_reserve_hpet_table(AcpiSdTableHeader* header)
{
    g_hpet_table = (AcpiHpetTable*)header;

    KLOGF(INFO, "Reserving HPET table at 0x%p.", (void*)header);
    mmanager_reserve_acpi_range((ptr)g_hpet_table, sizeof(AcpiHpetTable));
}

_INIT bool acpi_reserve_tables()
{
    AcpiRsdPointer* rsdp = acpi_find_rsdp();

    if (!acpi_is_rsdp_valid(rsdp))
    {
        KLOGF(WARN, "RSDP at 0x%p is invalid!", rsdp);
        return false;
    }

    g_rsd_table = (AcpiRsdTable*)rsdp->rsdp_v1.rsdt_base;

    KLOGF(INFO, "Reserving RSD table at 0x%p.", g_rsd_table);
    mmanager_reserve_acpi_range((ptr)g_rsd_table, sizeof(AcpiRsdTable));

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
                acpi_reserve_hpet_table(header);
        }
        else
            KLOGF(WARN, "Found invalid header at 0x%p.", header);

        header = (void*)header + header->len;
    }

    return true;
}

volatile AcpiHpetTable* acpi_get_hpet_table()
{
    return g_hpet_table;
}
