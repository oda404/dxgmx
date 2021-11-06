/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/acpi.h>
#include<dxgmx/string.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/attrs.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "acpi: " fmt, ##__VA_ARGS__)

_INIT static ACPIRSDP *acpi_find_rsdp()
{
    u32 ebda_hopefully = 0;
    memcpy(&ebda_hopefully, (void *)0x40E, 2);
    ebda_hopefully <<= 4;

    for(size_t i = ebda_hopefully; i < ebda_hopefully + 0x400; i += 16)
    {
        if(memcmp((void *)i, "RSD PTR ", 8) == 0)
            return (ACPIRSDP *)i;
    }

    for(size_t i = 0xE0000; i < 0xFFFFF; i += 16)
    {
        if(memcmp((void *)i, "RSD PTR ", 8) == 0)
            return (ACPIRSDP *)i;
    }

    return NULL;
}

_INIT static int acpi_is_rsdp_valid(const ACPIRSDP *rsdp)
{
    if(!rsdp)
        return 0;

    u32 sum = 0;

    for(size_t i = 0; i < sizeof(ACPIRSDPV1); ++i)
    {
        sum += *((const u8 *)rsdp + i);
    }

    if(rsdp->rsdp_v1.rev == 2)
    {
        for(size_t i = sizeof(ACPIRSDPV1); i < sizeof(ACPIRSDPV2); ++i)
        {
            sum += *((const u8 *)rsdp + i);
        }
    }

    return !(sum & 0xFF);
}

_INIT static int acpi_is_sdt_header_valid(const ACPISDTHeader *header)
{
    u32 sum = 0;

    for(size_t i = 0; i < header->len; ++i)
    {
        sum += *((const u8 *)header + i);
    }

    return !(sum & 0xFF);
}

static volatile ACPIHPETT *g_hpett = NULL;

_INIT static void acpi_parse_hpet(ACPISDTHeader *header)
{
    KLOGF(INFO, "Reserving HPET table at " PTR_FMT ".\n", (ptr)header);
    mmanager_reserve_acpi_range((ptr)header, sizeof(ACPIHPETT));
    g_hpett = (ACPIHPETT*)header;
}

volatile ACPIHPETT* acpi_get_hpett()
{
    return g_hpett;
}

static const ACPIRSDT *g_rsdt = NULL;

_INIT int acpi_reserve_tables()
{
    ACPIRSDP *rsdp = acpi_find_rsdp();

    if(!acpi_is_rsdp_valid(rsdp))
        abandon_ship("ACPI: RSDP at 0x%lX is invalid. Not proceeding.\n", (u32)g_rsdt);

    g_rsdt = (ACPIRSDT *)rsdp->rsdp_v1.rsdt_base;
    if(!acpi_is_sdt_header_valid(&g_rsdt->header))
        abandon_ship("ACPI: RSDT at 0x%lX is invalid. Not proceeding.\n", (u32)g_rsdt);

    size_t rsdt_max_tables = (g_rsdt->header.len - sizeof(g_rsdt->header)) / 4;

    u32 offset = 0;
    for(size_t i = 0; i < rsdt_max_tables; ++i)
    {
        ACPISDTHeader *header = ((void *)(g_rsdt->tables) + offset);
        offset += header->len;

        if(acpi_is_sdt_header_valid(header))
        {
            if(memcmp(header->signature, "HPET", 4) == 0)
                acpi_parse_hpet(header);
        }
        else
            KLOGF(WARN, "Found invalid header at 0x%lX.\n", (u32)header);
    }

    return 0;
}
