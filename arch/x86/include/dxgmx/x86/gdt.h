/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_GDT_H
#define _DXGMX_X86_GDT_H

#ifndef _ASM

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

typedef struct _ATTR_PACKED S_GDTEntry
{
    /* First 16 bits of the limit. */
    u16 limit_lo;
    /* First 24 bits of the base. */
    u32 base_lo : 24;
    union
    {
        u8 access_byte;
        struct
        {
            /* Set to 1 by the CPU if the segment was accessed. */
            u8 accessed : 1;
            /* 1 if the segment is read/write, 0 if the segment is read-only. */
            u8 rw : 1;
            /* I don't even know, look it up. */
            u8 direction_conforming : 1;
            /* 1 if the segment is executable (code segment)  */
            u8 exec : 1;
            /* 1 for a code or data segment, 0 for a system segment (tss/ldt) */
            u8 code_or_data : 1;
            /* Privilege level. */
            u8 dpl : 2;
            /* 1 for a valid segment. */
            u8 present : 1;
        };
    };

    /* Bits 16-19 of the limit. */
    u8 limit_hi : 4;
    /* Must be 0. */
    u8 reserved : 1;
    /* If the segment is for long mode. See GDT_SEG_LONG. */
    u8 longmode : 1;
    /* 0 for a 16 bit segment, 1 for a 32bit segment. */
    u8 bits32 : 1;
    /* See GDT_SEG_GRANULARITY */
    u8 granularity : 1;
    /* Bits 24-31 of the base. */
    u8 base_hi;
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

void gdt_init();
void tss_init();

void tss_set_esp0(ptr esp);
ptr tss_get_esp0();

#endif // !_ASM

/* The ring 0 mode code segment */
#define GDT_KERNEL_CS 0x8
/* The ring 0 data segment. */
#define GDT_KERNEL_DS 0x10
/* The ring  3 code segment. */
#define GDT_USER_CS 0x18
/* The ring 3 data segment */
#define GDT_USER_DS 0x20
#define GDT_TSS 0x28

#endif // _DXGMX_X86_GDT_H
