/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_ACPI_TABLES_H
#define _DXGMX_ACPI_TABLES_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_ACPIGenericAddress
{
#define ACPI_GAS_MEM 0
#define ACPI_GAS_IO 1
    u8 address_space_id;
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 reserved;
    u64 address;
} ACPIGenericAddress;

/* Root system descriptor pointer v1 */
typedef struct _ATTR_PACKED S_ACPIRSDPointerV1
{
    u8 signature[8];
    u8 checksum;
    u8 oem_id[6];
    u8 rev;
    u32 rsdt_base;
} ACPIRSDPointerV1;

/* Root system descriptor pointer v2 */
typedef struct _ATTR_PACKED S_ACPIRSDPointerV2
{
    ACPIRSDPointerV1 rsdpv1;
    u32 len;
    u64 xsdt_base;
    u8 ext_checksum;
    u8 reserved[3];
} ACPIRSDPointerV2;

typedef ACPIRSDPointerV2 ACPIRSDPointer;

/* System descriptor table header */
typedef struct _ATTR_PACKED S_ACPISDTHeader
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
} ACPISDTHeader;

typedef struct _ATTR_PACKED S_ACPIHPETTable
{
    ACPISDTHeader header;
    u8 hw_rev_id;
    u8 comparator_count : 5;
    u8 counter_size : 1;
    u8 reserved : 1;
    u8 legacy_replacement : 1;
    u16 pci_vendor_id;
    ACPIGenericAddress address;
    u8 hpet_number;
    u16 min_tick;
    u8 page_prot;
} ACPIHPETTable;

typedef struct _ATTR_PACKED S_ACPIMADTable
{
    ACPISDTHeader header;
    u32 lapic_address;
    u32 flags;
} ACPIMADTable;

/* System descriptor table */
typedef struct _ATTR_PACKED S_ACPIRSDTable
{
    ACPISDTHeader header;
    u32 tables[];
} ACPIRSDTable;

typedef enum E_ACPITableType
{
    /* In no particular order */
    ACPI_TABLE_HPET = 1,
    ACPI_TABLE_APIC,
    ACPI_TABLE_FACP,
    ACPI_TABLE_WAET
} ACPITableType;

ACPIHPETTable* acpi_get_hpet_table();
ACPIMADTable* acpi_get_mad_table();

#endif // !_DXGMX_ACPI_TABLES_H
