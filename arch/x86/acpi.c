/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/acpi.h>
#include<dxgmx/string.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<stddef.h>

static ACPIRSDP *acpi_find_rsdp()
{
    uint32_t ebda_hopefully = 0;
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

static int acpi_is_rsdp_valid(const ACPIRSDP *rsdp)
{
    if(!rsdp)
        return 0;

    uint32_t sum = 0;

    for(size_t i = 0; i < sizeof(ACPIRSDPV1); ++i)
    {
        sum += *((uint8_t *)rsdp + i);
    }

    if(rsdp->rsdp_v1.rev == 2)
    {
        for(size_t i = sizeof(ACPIRSDPV1); i < sizeof(ACPIRSDPV2); ++i)
        {
            sum += *((uint8_t *)rsdp + i);
        }
    }

    return !(sum & 0xFF);
}

static int acpi_is_sdt_header_valid(const ACPISDTHeader *header)
{
    uint32_t sum = 0;

    for(size_t i = 0; i < header->len; ++i)
    {
        sum += *((uint8_t *)header + i);
    }

    return !(sum & 0xFF);
}

static ACPIHPETT *g_hpett = NULL;

static void acpi_parse_hpet(ACPISDTHeader *header)
{
    g_hpett = (ACPIHPETT*)header;
}

ACPIHPETT* acpi_get_hpett()
{
    return g_hpett;
}

int acpi_init()
{
    ACPIRSDP *rsdp = acpi_find_rsdp();

    if(acpi_is_rsdp_valid(rsdp))
        klog(KLOG_INFO, "[ACPI] Found RSDP at 0x%lX.\n", (uint32_t)rsdp);
    else
        abandon_ship("[ACPI] RSDP at 0x%lX is invalid. Not proceeding.\n", (uint32_t)rsdp);

    ACPIRSDT *rsdt = (ACPIRSDT *)rsdp->rsdp_v1.rsdt_base;
    if(!acpi_is_sdt_header_valid(&rsdt->header))
        abandon_ship("[ACPI] RSDT at 0x%lX is invalid. Not proceeding.\n", (uint32_t)rsdt);

    size_t rsdt_max_tables = (rsdt->header.len - sizeof(rsdt->header)) / 4;

    uint32_t offset = 0;
    for(size_t i = 0; i < rsdt_max_tables; ++i)
    {
        ACPISDTHeader *header = ((void *)(rsdt->tables) + offset);
        offset += header->len;

        if(acpi_is_sdt_header_valid(header))
        {
            klog(KLOG_INFO, "[ACPI] Found header '%.4s'.\n", header->signature);

            if(memcmp(header->signature, "HPET", 4) == 0)
                acpi_parse_hpet(header);
        }
        else
        {
            klog(KLOG_WARN, "[ACPI] Found invalid header at 0x%lX.\n", (uint32_t)header);
        }
    }

    return 0;
}
