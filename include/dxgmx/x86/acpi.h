/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_ACPI_H
#define _DXGMX_X86_ACPI_H

#include<dxgmx/attrs.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/types.h>

typedef struct
_ATTR_PACKED S_ACPIGenericAddress
{
#define ACPI_GAS_ADDR_SPACE_ID_MEM 0
#define ACPI_GAS_ADDR_SPACE_ID_IO  1
    u8 address_space_id;
    u8 register_bit_width;
    u8 register_bit_offset;
    u8 reserved;
    u64 address;
} ACPIGenericAddress;

typedef struct
_ATTR_PACKED S_ACPIRSDPV1
{
    u8 signature[8];
    u8 checksum;
    u8 oem_id[6];
    u8 rev;
    u32 rsdt_base;
} ACPIRSDPV1;

typedef struct
_ATTR_PACKED S_ACPIRSDPV2
{
    ACPIRSDPV1 rsdp_v1;
    u32 len;
    u64 xsdt_base;
    u8 ext_checksum;
    u8 reserved[3];
} ACPIRSDPV2;

typedef ACPIRSDPV2 ACPIRSDP;

typedef struct 
_ATTR_PACKED S_ACPISDTHeader
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

typedef struct 
_ATTR_PACKED S_ACPIHPETT
{
    ACPISDTHeader      header;
    u8        hw_rev_id;
    u8        comparator_count:   5;
    u8        counter_size:       1;
    u8        reserved:           1;
    u8        legacy_replacement: 1;
    uint16_t       pci_vendor_id;
    ACPIGenericAddress address;
    u8        hpet_number;
    uint16_t       min_tick;
    u8        page_prot;
} ACPIHPETT;

typedef struct
_ATTR_PACKED S_ACPIRSDT
{
    ACPISDTHeader header;
    u32 *tables;
} ACPIRSDT;

int acpi_reserve_tables();
volatile ACPIHPETT *acpi_get_hpett();

#endif //_DXGMX_X86_ACPI_H
