/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/acpi_tables.h>
#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/module.h>
#include <dxgmx/panic.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/bytes.h>

#define KLOGF_PREFIX "acpi: "

typedef struct S_ACPITable
{
    ACPITableType type;
    ptr vaddr;
} ACPITable;

static ACPITable* g_acpi_tables;
static size_t g_acpi_table_count;

static ACPITable* acpi_new_table()
{
    ACPITable* tmp =
        krealloc(g_acpi_tables, (g_acpi_table_count + 1) * sizeof(ACPITable));

    if (!tmp)
        return NULL;

    g_acpi_tables = tmp;
    ++g_acpi_table_count;

    return &g_acpi_tables[g_acpi_table_count - 1];
}

static _INIT ACPIRSDPointer* acpi_find_rsdp()
{
    /* The RSDP is under the first MiB of physical memory, which is higher half
     * mapped along with the kernel image, so no worries here. */

    u32 ebda = *(u16*)(0x40E + kimg_map_offset()) << 4;
    ebda += kimg_map_offset();

    for (ptr i = ebda; i < ebda + 0x400; i += 16)
    {
        if (memcmp((void*)i, "RSD PTR ", 8) == 0)
            return (ACPIRSDPointer*)i;
    }

    for (ptr i = 0xE0000 + kimg_map_offset(); i < 0x100000 + kimg_map_offset();
         i += 16)
    {
        if (memcmp((void*)i, "RSD PTR ", 8) == 0)
            return (ACPIRSDPointer*)i;
    }

    return NULL;
}

static _INIT bool acpi_is_rsdp_valid(const ACPIRSDPointer* rsdp)
{
    if (!rsdp)
        return false;

    u32 sum = 0;
    for (size_t i = 0; i < sizeof(ACPIRSDPointerV1); ++i)
        sum += *((const u8*)rsdp + i);

    if (rsdp->rsdpv1.rev == 2)
    {
        for (size_t i = sizeof(ACPIRSDPointerV1); i < sizeof(ACPIRSDPointerV2);
             ++i)
            sum += *((const u8*)rsdp + i);
    }

    return !(sum & 0xFF);
}

static _INIT bool acpi_is_sdt_header_valid(const ACPISDTHeader* header)
{
    if (!header)
        return false;

    u32 sum = 0;
    for (size_t i = 0; i < header->len; ++i)
        sum += *((const u8*)header + i);

    return !(sum & 0xFF);
}

static int acpi_register_table(ACPISDTHeader* hdr)
{
    ACPITable table;

    if (memcmp(hdr->signature, "HPET", 4) == 0)
        table.type = ACPI_TABLE_HPET;
    else if (memcmp(hdr->signature, "FACP", 4) == 0)
        table.type = ACPI_TABLE_FACP;
    else if (memcmp(hdr->signature, "APIC", 4) == 0)
        table.type = ACPI_TABLE_APIC;
    else if (memcmp(hdr->signature, "WAET", 4) == 0)
        table.type = ACPI_TABLE_WAET;
    else
    {
        KLOGF(WARN, "Unknown table type: %.4s", hdr->signature);
        return -EINVAL;
    }

    table.vaddr = (ptr)hdr;

    ACPITable* tmp = acpi_new_table();
    if (!tmp)
        return -ENOMEM;

    *tmp = table;

    KLOGF(INFO, "%.4s at 0x%p.", hdr->signature, (void*)hdr);

    return 0;
}

static ERR_OR(ptr) acpi_map_table(ptr hdr_paddr)
{
#define MIN_MAP (PAGESIZE)

    /* We map the minimum we can, just so we can access the header. */
    ERR_OR(ptr) res = dma_map_range(hdr_paddr, MIN_MAP, PAGE_RW);
    if (res.error)
        return ERR(ptr, res.error);

    ACPISDTHeader* hdr = (ACPISDTHeader*)res.value;
    if (hdr->len > MIN_MAP)
    {
        res = dma_map_range(hdr_paddr, hdr->len, PAGE_RW);
        if (res.error)
            return ERR(ptr, res.error); // FIXME: free the initial allocation
    }

#undef MIN_MAP

    return VALUE(ptr, res.value);
}

static int acpi_enumerate_rsd_tables(ACPIRSDTable* rsdt)
{
    const size_t tables = (rsdt->header.len - sizeof(rsdt->header)) / 4;

    for (size_t i = 0; i < tables; ++i)
    {
        ptr hdr_paddr = rsdt->tables[i];
        ERR_OR(ptr) res = acpi_map_table(hdr_paddr);
        if (res.error)
            return res.error;

        ACPISDTHeader* hdr = (ACPISDTHeader*)res.value;
        if (!acpi_is_sdt_header_valid(hdr))
        {
            KLOGF(ERR, "Invalid table at 0x%p!", (void*)hdr_paddr);
            continue;
        }

        acpi_register_table(hdr);
    }

    return 0;
}

static int acpi_init_rsdt()
{
    ACPIRSDPointer* rsdp = acpi_find_rsdp();
    if (!acpi_is_rsdp_valid(rsdp))
    {
        KLOGF(WARN, "RSDP at 0x%p is invalid!", rsdp);
        return -EINVAL;
    }

    ERR_OR(ptr) res = acpi_map_table(rsdp->rsdpv1.rsdt_base);
    if (res.error)
        return res.error;

    ACPIRSDTable* rsdt = (ACPIRSDTable*)res.value;
    if (!acpi_is_sdt_header_valid(&rsdt->header))
    {
        KLOGF(WARN, "RSDT at 0x%p is invalid!", rsdt);
        return -EINVAL;
    }

    KLOGF(INFO, "RSDT at 0x%p.", rsdt);

    int st = acpi_enumerate_rsd_tables(rsdt);
    if (st < 0)
        return st;

    return 0;
}

ACPIHPETTable* acpi_get_hpet_table()
{
    FOR_EACH_ELEM_IN_DARR (g_acpi_tables, g_acpi_table_count, table)
    {
        if (table->type == ACPI_TABLE_HPET)
            return (ACPIHPETTable*)table->vaddr;
    }

    return NULL;
}

static int acpi_main()
{
    acpi_init_rsdt();
    return 0;
}

static int acpi_exit()
{
    return 0;
}

MODULE g_acpi_module = {
    .name = "acpi",
    .main = acpi_main,
    .exit = acpi_exit,
    .stage = MODULE_STAGE2};
