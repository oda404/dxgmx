/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_GDT_H
#define _DXGMX_X86_GDT_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_GDTEntry
{
    /* First 16 bits of the limit. */
    u16 limit_0_15;
    /* First 16 bits of the base. */
    u16 base_0_15;
    /* Bits 16-23 of the base. */
    u8 base_16_23;
    /* Set to one by the cpu if the segment has been accessed. */
    union
    {
        u8 access_byte;
        struct
        {
            u8 access_accessed : 1;
            u8 access_rw : 1;
            u8 access_dc : 1;
            u8 access_exec : 1;
            u8 access_type : 1;
            u8 access_dpl : 2;
            u8 access_present : 1;
        };
    };
    /* Bits 16-19 of the limit. */
    u8 limit_16_19 : 4;
    /* Must be 0. */
    u8 reserved : 1;
    /* If the segment is for long mode. See GDT_SEG_LONG. */
    u8 longmode : 1;
    /* See GDT_SEG_SIZE. */
    u8 size : 1;
    /* See GDT_SEG_GRANULARITY */
    u8 granularity : 1;
    /* Bits 24-31 of the base. */
    u8 base_24_31;
} GDTEntry;

typedef struct _ATTR_PACKED S_TssEntry
{
    u32 prev_tss;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldt;
    u16 trap;
    u16 iomap_base;
} TssEntry;

typedef struct _ATTR_PACKED S_GDTR
{
    u16 limit;
    GDTEntry* base;
} GDTR;

/* The ring 0 mode code segment */
#define GDT_KERNEL_CS 0x8
/* The ring 0 data segment. */
#define GDT_KERNEL_DS 0x10
/* The ring  3 code segment. */
#define GDT_USER_CS 0x18
/* The ring 3 data segment */
#define GDT_USER_DS 0x20
#define GDT_TSS 0x28

void gdt_init();
void tss_init();

#endif // _DXGMX_X86_GDT_H
