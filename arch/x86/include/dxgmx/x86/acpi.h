/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_ACPI_H
#define _DXGMX_X86_ACPI_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_AcpiGenericAddress
{
#define ACPI_GAS_ADDR_SPACE_ID_MEM 0
#define ACPI_GAS_ADDR_SPACE_ID_IO 1
    u8 address_space_id;
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 reserved;
    u64 address;
} AcpiGenericAddress;

typedef struct _ATTR_PACKED S_AcpiRsdPointerV1
{
    u8 signature[8];
    u8 checksum;
    u8 oem_id[6];
    u8 rev;
    u32 rsdt_base;
} AcpiRsdPointerV1;

typedef struct _ATTR_PACKED S_AcpiRsdPointerV2
{
    AcpiRsdPointerV1 rsdp_v1;
    u32 len;
    u64 xsdt_base;
    u8 ext_checksum;
    u8 reserved[3];
} AcpiRsdPointerV2;

typedef AcpiRsdPointerV2 AcpiRsdPointer;

typedef struct _ATTR_PACKED S_AcpiSdTableHeader
{
    u8 signature[4];
    u32 len;
    u8 rev;
    u8 checksum;
    u8 oem_id[6];
    u8 oem_table_id[8];
    u32 oem_revision;
    u32 creator_id;
    u32 creator_revision;
} AcpiSdTableHeader;

typedef struct _ATTR_PACKED S_AcpiHpetTable
{
    AcpiSdTableHeader header;
    u8 hw_rev_id;
    u8 comparator_count : 5;
    u8 counter_size : 1;
    u8 reserved : 1;
    u8 legacy_replacement : 1;
    u16 pci_vendor_id;
    AcpiGenericAddress address;
    u8 hpet_number;
    u16 min_tick;
    u8 page_prot;
} AcpiHpetTable;

typedef struct _ATTR_PACKED S_AcpiRsdTable
{
    AcpiSdTableHeader header;
    u32* tables;
} AcpiRsdTable;

bool acpi_reserve_tables();
volatile AcpiHpetTable* acpi_get_hpet_table();

#endif //_DXGMX_X86_ACPI_H
