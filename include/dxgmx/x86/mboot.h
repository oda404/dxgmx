
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

#endif // __ASM__

#endif // __DXGMX_MBOOT_H__
