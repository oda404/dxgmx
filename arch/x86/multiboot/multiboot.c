/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/multiboot.h>

#define FLAGS                                                                  \
    (MULTIBOOT_FLAG_MOD_PAGE_ALIGN | MULTIBOOT_FLAG_MEM_INFO |                 \
     MULTIBOOT_FLAG_VIDEO)
#define CHECKSUM -(MULTIBOOT_HEADER_MAGIC + FLAGS)
#define HEADER_ADDR 0
#define LOAD_ADDR 0
#define LOAD_END_ADDR 0
#define BSS_END_ADDR 0
#define ENTRY_ADDR 0
#define VIDEO_MODE_TYPE 0
#define VIDEO_WIDTH 800
#define VIDEO_HEIGHT 600
#define VIDEO_DEPTH 32

_ATTR_SECTION(".bootloader")
long mbootconfig[] = {
    MULTIBOOT_HEADER_MAGIC,
    FLAGS,
    CHECKSUM,
    HEADER_ADDR,
    LOAD_ADDR,
    LOAD_END_ADDR,
    BSS_END_ADDR,
    ENTRY_ADDR,
    VIDEO_MODE_TYPE,
    VIDEO_WIDTH,
    VIDEO_HEIGHT,
    VIDEO_DEPTH};

const ptr _multiboot_magic = 0;
const u32 _multiboot_info_struct_base = 0;
