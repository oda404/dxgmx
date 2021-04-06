
#ifndef __DXGMX_MBOOT2_H__
#define __DXGMX_MBOOT2_H__

#define MBOOT2_HEADER_ALIGN      8
#define MBOOT2_HEADER_MAGIC      0xE85250D6
#define MBOOT2_BOOTLOADER_MAGIC  0x36D76289

#define MBOOT2_HEADER_TAG_END    0
#define MBOOT2_HEADER_TAG_OPT    1

#define MBOOT2_TAG_MMAP          6
#define MBOOT2_TAG_FRAMEBUFFER   8

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
} mboot_tag;

#endif // __ASM__

#endif // __DXGMX_MBOOT2_H__
