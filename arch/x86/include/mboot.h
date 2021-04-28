/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_MBOOT_H__
#define __DXGMX_MBOOT_H__

#define MBOOT_HEADER_ALIGN      4
#define MBOOT_HEADER_MAGIC      0x1BADB002
/* this is what the bootloader slaps in %eax to prove we are mbooting */
#define MBOOT_BOOTLOADER_MAGIC  0x2BADB002

/* align modules on 4K page boundaries */
#define MBOOT_FLAG_PAGE_ALIGN   0x1
/* provide the kernel with info about the memory */
#define MBOOT_FLAG_MEM_INFO     0x2
/* provide the kernel with info about the video  */
#define MBOOT_FLAG_VIDEO_INFO   0x4

#ifndef __ASM__

#include<dxgmx/gcc/attrs.h>
#include<stdint.h>

typedef struct
__ATTR_PACKED
{
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_base_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_base_addr;
    uint32_t drives_length;
    uint32_t drives_base_addr;
    uint32_t config_table;
    uint32_t bl_name_base_addr;
    uint32_t apm_table_base_addr;

    //...
} mboot_mbi;

#define MBOOT_MMAP_TYPE_AVAILABLE        1
#define MBOOT_MMAP_TYPE_RESERVED         2
#define MBOOT_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MBOOT_MMAP_TYPE_NVS              4
#define MBOOT_MMAP_TYPE_BADRAM           5

typedef struct
__ATTR_PACKED
{
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} mboot_mmap;

#endif // __ASM__

#endif // __DXGMX_MBOOT_H__
