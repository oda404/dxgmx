/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef __DXGMX_MBOOT2_H__
#define __DXGMX_MBOOT2_H__

#define MBOOT2_HEADER_ALIGN      8
#define MBOOT2_HEADER_MAGIC      0xE85250D6
#define MBOOT2_BOOTLOADER_MAGIC  0x36D76289

#define MBOOT2_HEADER_TAG_END       0
#define MBOOT2_HEADER_TAG_OPT       1
#define MBOOT2_HEADER_TAG_FRAMEBUFF 5

#define MBOOT2_TAG_END           0
#define MBOOT2_TAG_BOOTCLI       1
#define MBOOT2_TAG_MMAP          6
#define MBOOT2_TAG_FRAMEBUFFER   8
#define MBOOT2_TAG_IMG_BASE_ADDR 21

#define MBOOT2_TAGS_ALIGN        8

#define MBOOT2_ARCH_I386         0

#ifndef __ASM__

#include<stdint.h>

#define MBOOT2_MMAP_ENTRY_TYPE_AVAILABLE        1
#define MBOOT2_MMAP_ENTRY_TYPE_RESERVED         2  
#define MBOOT2_MMAP_ENTRY_TYPE_ACPI_RECLAIMABLE 3
#define MBOOT2_MMAP_ENTRY_TYPE_NVS              4
#define MBOOT2_MMAP_ENTRY_TYPE_BAD              5

typedef struct
{
    uint32_t type;
    uint32_t size;
} mboot2_mbi_tag_header;

typedef struct
{
    uint32_t total_size;
    uint32_t reserved;
} mboot2_mbi_fixed;

typedef struct
{
    mboot2_mbi_tag_header header;
    uint8_t *string; /* string passed from the boot cli */
} mboot2_mbi_tag_bootcli;

typedef struct
{
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} mboot2_mmap_entry __attribute__((packed));

typedef struct
{
    mboot2_mbi_tag_header header;
    uint32_t entry_size;
    uint32_t entry_version;
    mboot2_mmap_entry *entries;
} mboot2_mbi_tag_mmap;

typedef struct
{
    mboot2_mbi_tag_header header;
    uint32_t base_addr;
} mboot2_mbi_tag_img_base_addr;

#endif // __ASM__

#endif // __DXGMX_MBOOT2_H__
