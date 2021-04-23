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

#define U32 __UINT32_TYPE__
#define U64 __UINT64_TYPE__

__attribute__((packed))
typedef struct
{
    U32 flags;
    U32 mem_lower;
    U32 mem_upper;
    U32 boot_device;
    U32 cmdline;
    U32 mods_count;
    U32 mods_base_addr;
    U32 syms[4];
    U32 mmap_length;
    U32 mmap_base_addr;
    U32 drives_length;
    U32 drives_base_addr;
    U32 config_table;
    U32 bl_name_base_addr;
    U32 apm_table_base_addr;

    //...
} mboot_mbi;

#define MBOOT_MMAP_TYPE_AVAILABLE        1
#define MBOOT_MMAP_TYPE_RESERVED         2
#define MBOOT_MMAP_TYPE_ACPI_RECLAIMABLE 3
#define MBOOT_MMAP_TYPE_NVS              4
#define MBOOT_MMAP_TYPE_BADRAM           5

__attribute__((packed))
typedef struct
{
    U32 size;
    U64 base_addr;
    U64 length;
    U32 type;
} mboot_mmap;

#endif // __ASM__

#endif // __DXGMX_MBOOT_H__
