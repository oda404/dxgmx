/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MULTIBOOT_H
#define _DXGMX_MULTIBOOT_H

#define MULTIBOOT_HEADER_ALIGN 4
#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
/* this is what the bootloader slaps in %eax to prove we are mbooting */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/* align modules on 4K page boundaries */
#define MULTIBOOT_FLAG_MOD_PAGE_ALIGN 0x1
/* provide the kernel with info about the memory */
#define MULTIBOOT_FLAG_MEM_INFO 0x2
/* provide the kernel with info about the video  */
#define MULTIBOOT_FLAG_VIDEO 0x4

#ifndef _ASM

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/kboot.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_MultibootMBI
{
    u32 flags;
    u32 mem_lower;
    u32 mem_upper;
    u32 boot_device;
    u32 cmdline;
    u32 mods_count;
    u32 mods_base;
    u32 syms[4];
    u32 mmap_length;
    u32 mmap_base;
    u32 drives_length;
    u32 drives_base;
    u32 config_table;
    u32 bootloader_name_base;
    u32 apm_table_base;

    struct
    {
        u32 control_info;
        u32 mode_info;
        u16 mode;
        u16 interface_seg;
        u16 interface_off;
        u16 interface_len;
    } vbe;

    struct
    {
        u64 base;
        u32 pitch;
        u32 width;
        u32 height;
        u8 bpp;
        u8 type;
        /**
         * Since this is an union only one of these structs will be valid at a
         * time. If MultibootMBI::fb::type is 0, the first struct will be valid.
         * Else if MultibootMBI::fb::type is 1, the second struct will be valid.
         * Else if MultibootMBI::fb::type is 2, EGA standard text mode is used
         * and this union is obsolete.
         */
        union
        {
            struct
            {
                u32 pallete_base;
                u16 pallete_colors_count;
            };
            struct
            {
                u8 red_field_pos;
                u8 red_mask_size;
                u8 green_field_pos;
                u8 green_mask_size;
                u8 blue_field_pos;
                u8 blue_mask_size;
            };
        };
    } fb;

} MultibootMBI;

#define MULTIBOOT_MMAP_TYPE_AVAILABLE 1
#define MULTIBOOT_MMAP_TYPE_RESERVED 2
#define MULTIBOOT_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MMAP_TYPE_NVS 4
#define MULTIBOOT_MMAP_TYPE_BADRAM 5

typedef struct _ATTR_PACKED S_MultibootMMAP
{
    u32 size;
    u64 base;
    u64 length;
    u32 type;
} MultibootMMAP;

extern const u32 ___multiboot_magic;
extern const ptr ___multiboot_struct_pa;

int multiboot_parse_info(KernelBootInfo* kbootinfo);

#endif // _ASM

#endif // _DXGMX_MULTIBOOT_H
