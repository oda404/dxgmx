
#include<dxgmx/x86/acpi.h>
#include<dxgmx/string.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/klog.h>
#include<stddef.h>

RSDP *acpi_find_rsdp()
{
    uint32_t ebda_hopefully = 0;
    memcpy(&ebda_hopefully, (void *)0x40E, 2);
    ebda_hopefully <<= 4;

    for(size_t i = ebda_hopefully; i < ebda_hopefully + 0x400; i += 16)
    {
        if(memcmp((void *)i, "RSD PTR ", 8) == 0)
            return (RSDP *)i;
    }

    for(size_t i = 0xE0000; i < 0xFFFFF; i += 16)
    {
        if(memcmp((void *)i, "RSD PTR ", 8) == 0)
            return (RSDP *)i;
    }

    return NULL;
}

int acpi_is_rsdp_valid(const RSDP *rsdp)
{
    if(!rsdp)
        return 0;

    uint32_t sum = 0;

    for(size_t i = 0; i < sizeof(RSDPV1); ++i)
    {
        sum += *((uint8_t *)rsdp + i);
    }

    if(rsdp->rsdp_v1.revision == 2)
    {
        for(size_t i = sizeof(RSDPV1); i < sizeof(RSDPV2); ++i)
        {
            sum += *((uint8_t *)rsdp + i);
        }
    }

    return !(sum & 0xFF);
}

int acpi_is_sdt_header_valid(const SDTHeader *header)
{
    uint32_t sum = 0;

    for(size_t i = 0; i < header->len; ++i)
    {
        sum += *((uint8_t *)header + i);
    }

    return !(sum & 0xFF);
}

int acpi_init()
{
    RSDP *rsdp = acpi_find_rsdp();

    if(acpi_is_rsdp_valid(rsdp))
        klog(KLOG_INFO, "[ACPI] Found RSDP at 0x%lX.\n", (uint32_t)rsdp);
    else
        abandon_ship("[ACPI] RSDP at 0x%lX is invalid. Not proceeding.\n", (uint32_t)rsdp);

    RSDT *rsdt = (RSDT *)rsdp->rsdp_v1.rsdt_base;
    if(!acpi_is_sdt_header_valid(&rsdt->header))
        abandon_ship("[ACPI] RSDT at 0x%lX is invalid. Not proceeding.\n", (uint32_t)rsdt);

    size_t rsdt_max_tables = (rsdt->header.len - sizeof(rsdt->header)) / 4;

    uint32_t offset = 0;
    for(size_t i = 0; i < rsdt_max_tables; ++i)
    {
        SDTHeader *header = ((void *)(rsdt->tables) + offset);
        offset += header->len;

        char sig[5] = { 0 };
        memcpy(sig, header->signature, 4);

        if(acpi_is_sdt_header_valid(header))
            klog(KLOG_INFO, "[ACPI] Found header '%s'.\n", sig);
        else
            klog(KLOG_WARN, "[ACPI] Found invalid header at 0x%lX.\n", (uint32_t)header);
    }

    return 0;
}
