/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/kimg.h>
#include <dxgmx/multiboot.h>
#include <dxgmx/panic.h>
#include <dxgmx/types.h>

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

const volatile ptr ___multiboot_magic;
const volatile u32 ___multiboot_struct_pa;

int multiboot_parse_info(KernelBootInfo* kbootinfo)
{
    if (___multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
        return -1;

    MultibootMBI* mbi =
        (MultibootMBI*)(___multiboot_struct_pa + kimg_map_offset());

    if (mbi->fb.type == 1)
    {
        kbootinfo->has_fb = true;
        kbootinfo->fb_pa = mbi->fb.base;
        kbootinfo->fb_width = mbi->fb.width;
        kbootinfo->fb_height = mbi->fb.height;
        kbootinfo->fb_bpp = mbi->fb.bpp;
    }

    MemoryRegionMap* mregmap = kbootinfo->mregmap;
    for (MultibootMMAP* mmap =
             (MultibootMMAP*)(mbi->mmap_base + kimg_map_offset());
         (ptr)mmap < mbi->mmap_base + kimg_map_offset() + mbi->mmap_length;
         mmap = (MultibootMMAP*)((ptr)mmap + mmap->size + sizeof(mmap->size)))
    {
        if (mregmap->regions_size >= mregmap->regions_capacity)
            panic("Hit maximum number of system memory regions!");

        if (mmap->type != MULTIBOOT_MMAP_TYPE_AVAILABLE)
            continue;

        const MemoryRegion newreg = {
            .start = mmap->base, .size = mmap->length, .perms = MEM_REGION_RWX};
        mregmap_add_reg(&newreg, mregmap);
    }

    return 0;
}
