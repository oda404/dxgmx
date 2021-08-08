/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_MULTIBOOT_H
#define _DXGMX_X86_MULTIBOOT_H

#define MULTIBOOT_HEADER_ALIGN      4
#define MULTIBOOT_HEADER_MAGIC      0x1BADB002
/* this is what the bootloader slaps in %eax to prove we are mbooting */
#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

/* align modules on 4K page boundaries */
#define MULTIBOOT_FLAG_MOD_PAGE_ALIGN   0x1
/* provide the kernel with info about the memory */
#define MULTIBOOT_FLAG_MEM_INFO         0x2
/* provide the kernel with info about the video  */
#define MULTIBOOT_FLAG_VIDEO            0x4

#ifndef _ASM

#include<dxgmx/attrs.h>
#include<stdint.h>

typedef struct
_ATTR_PACKED S_MultibootMBI
{
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_base;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_base;
    uint32_t drives_length;
    uint32_t drives_base;
    uint32_t config_table;
    uint32_t bootloader_name_base;
    uint32_t apm_table_base;
    
    struct
    {
        uint32_t control_info;
        uint32_t mode_info;
        uint16_t mode;
        uint16_t interface_seg;
        uint16_t interface_off;
        uint16_t interface_len;
    } vbe;

    struct 
    {
        uint64_t base;
        uint32_t pitch;
        uint32_t width;
        uint32_t height;
        uint8_t  bpp;
        uint8_t  type;
        /**
         * Since this is an union only one of these structs will be valid at a time.
         * If MultibootMBI::fb::type is 0, the first struct will be valid.
         * Else if MultibootMBI::fb::type is 1, the second struct will be valid.
         * Else if MultibootMBI::fb::type is 2, EGA standard text mode is used and this union is obsolete.
        */
        union
        {
            struct
            {
                uint32_t pallete_base;
                uint16_t pallete_colors_count;
            };
            struct
            {
                uint8_t red_field_pos;
                uint8_t red_mask_size;
                uint8_t green_field_pos;
                uint8_t green_mask_size;
                uint8_t blue_field_pos;
                uint8_t blue_mask_size;
            };
        };
    } fb;
    
} MultibootMBI;

#define MULTIBOOT_MMAP_TYPE_AVAILABLE        1
#define MULTIBOOT_MMAP_TYPE_RESERVED         2
#define MULTIBOOT_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MMAP_TYPE_NVS              4
#define MULTIBOOT_MMAP_TYPE_BADRAM           5

typedef struct
_ATTR_PACKED S_MultibootMMAP
{
    uint32_t size;
    uint64_t base;
    uint64_t length;
    uint32_t type;
} MultibootMMAP;

#endif // _ASM

#endif // _DXGMX_X86_MULTIBOOT_H
