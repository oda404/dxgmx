
#ifndef __DXGMX_MBOOT_H__
#define __DXGMX_MBOOT_H__

#define MBOOT2_HEADER_ALIGN     8
#define MBOOT2_HEADER_MAGIC     0xE85250D6
#define MBOOT2_BOOTLOADER_MAGIC 0x36D76289
#define MBOOT2_ARCH_I386        0

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

#include<stdint.h>

#define MBOOT2_MMAP_ENTRY_TYPE_AVAILABLE        1
#define MBOOT2_MMAP_ENTRY_TYPE_RESERVED         2  
#define MBOOT2_MMAP_ENTRY_TYPE_ACPI_RECLAIMABLE 3
#define MBOOT2_MMAP_ENTRY_TYPE_NVS              4
#define MBOOT2_MMAP_ENTRY_TYPE_BAD              5

typedef struct 
{
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} mboot_mmap_entry;

typedef struct
{
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    mboot_mmap_entry *entries;
} mboot_mmap;

struct mboot2_header
{
    uint32_t magic;
    uint32_t arch;
    uint32_t header_length;
    uint32_t checksum;
};

#endif // __ASM__

#endif // __DXGMX_MBOOT_H__
