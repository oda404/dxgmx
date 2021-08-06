
#ifndef _DXGMX_X86_ACPI_H
#define _DXGMX_X86_ACPI_H

#include<dxgmx/attrs.h>
#include<stdint.h>

typedef struct
_ATTR_PACKED S_ACPIGenericAddress
{
#define ACPI_GAS_ADDR_SPACE_ID_MEM 0
#define ACPI_GAS_ADDR_SPACE_ID_IO  1
    uint8_t address_space_id;
    uint8_t register_bit_width;
    uint8_t register_bit_offset;
    uint8_t reserved;
    uint64_t address;
} ACPIGenericAddress;

typedef struct
_ATTR_PACKED S_ACPIRSDPV1
{
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t rev;
    uint32_t rsdt_base;
} ACPIRSDPV1;

typedef struct
_ATTR_PACKED S_ACPIRSDPV2
{
    ACPIRSDPV1 rsdp_v1;
    uint32_t len;
    uint64_t xsdt_base;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} ACPIRSDPV2;

typedef ACPIRSDPV2 ACPIRSDP;

typedef struct 
_ATTR_PACKED S_ACPISDTHeader
{
    uint8_t signature[4];
    uint32_t len;
    uint8_t rev;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} ACPISDTHeader;

typedef struct 
_ATTR_PACKED S_ACPIHPETT
{
    ACPISDTHeader      header;
    uint8_t        hw_rev_id;
    uint8_t        comparator_count:   5;
    uint8_t        counter_size:       1;
    uint8_t        reserved:           1;
    uint8_t        legacy_replacement: 1;
    uint16_t       pci_vendor_id;
    ACPIGenericAddress address;
    uint8_t        hpet_number;
    uint16_t       min_tick;
    uint8_t        page_prot;
} ACPIHPETT;

typedef struct
_ATTR_PACKED S_ACPIRSDT
{
    ACPISDTHeader header;
    uint32_t *tables;
} ACPIRSDT;

int acpi_init();
ACPIHPETT *acpi_get_hpett();

#endif //_DXGMX_X86_ACPI_H
