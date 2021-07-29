
#ifndef _DXGMX_X86_ACPI_H
#define _DXGMX_X86_ACPI_H

#include<dxgmx/attrs.h>
#include<stdint.h>

typedef struct
_ATTR_PACKED S_RSDPV1
{
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt_base;
} RSDPV1;

typedef struct
_ATTR_PACKED S_RSDPV2
{
    RSDPV1 rsdp_v1;
    uint32_t len;
    uint64_t xsdt_base;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} RSDPV2;

typedef RSDPV2 RSDP;

typedef struct 
_ATTR_PACKED S_SDTHeader
{
    uint8_t signature[4];
    uint32_t len;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} SDTHeader;

typedef struct
_ATTR_PACKED S_RSDT
{
    SDTHeader header;
    uint32_t *tables;
} RSDT;

RSDP *acpi_find_rsdp();
int acpi_is_rsdp_valid(const RSDP *rsdp);
int acpi_is_sdt_header_valid(const SDTHeader *header);
int acpi_init();

#endif //_DXGMX_X86_ACPI_H
