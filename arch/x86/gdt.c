/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/gdt.h>
#include<dxgmx/types.h>
#include<dxgmx/attrs.h>

/* set to 1 by the cpu if 
 * the segment is accessed,
 * defined just for completeness
 */
#define GDT_SEG_ACCESSED       (1 << 0)
/* 1 for data/code segments, 0 otherwise. */
#define GDT_SEG_TYPE(n)        ((n & 1) << 4)
/* ring number 0, 1, 2, 3 */
#define GDT_SEG_PRIVILEGE(n)   ((n & 3) << 5)
/* Segment is present. */
#define GDT_SEG_PRESENT        (1 << 7)

/* read */
#define GDT_SEG_R              (0 << 1)
/* read, write */
#define GDT_SEG_RW             (0b001 << 1)
/* read, segment grows down */
#define GDT_SEG_R_GROW_DOWN    (0b010 << 1)
/* read, write, segment grows down */
#define GDT_SEG_RW_GROW_DOWN   (0b011 << 1)
/* execute */
#define GDT_SEG_X              (0b100 << 1)
/* read, execute */
#define GDT_SEG_RX             (0b101 << 1)
/* execute, conforming */
#define GDT_SEG_X_CONF         (0b110 << 1)
/* read, execute, conforming */
#define GDT_SEG_RX_CONF        (0b111 << 1)

/* 1 for Page (4KiB) granularity, 0 for byte granularity. */
#define GDT_SEG_GRANULARITY(n) ((n & 1) << 3)
/* 0 for real mode, 1 for protected mode. */
#define GDT_SEG_SIZE(n)        ((n & 1) << 2)
/* Long mode segment. */
#define GDT_SEG_LONG           (1 << 1)

_INIT static void gdt_load(const GDTR *gdtr)
{
    __asm__ volatile(
        "lgdt (%0)                \n"
        "ljmp %1, $reload_segs    \n"
        "                         \n"
        "reload_segs:             \n"
        "  movw %2, %%ax       \n"
        "  movw %%ax, %%ds        \n"
        "  movw %%ax, %%es        \n"
        "  movw %%ax, %%fs        \n"
        "  movw %%ax, %%gs        \n"
        "  movw %%ax, %%ss        \n"
        : 
        : "b"(gdtr), "i"(GDT_KERNEL_CS), "i"(GDT_KERNEL_DS)
    );
}

_INIT static void gdt_encode_entry(
    u32 base,
    u32 limit,
    u8 access_byte,
    u8 flags,
    GDTEntry *entry
)
{
    entry->limit_0_15  = (limit & 0xFFFF);
    entry->base_0_15   = (base & 0xFFFF);
    entry->base_16_23  = (base & 0xFF0000) >> 16;

    entry->accessed    = 0;
    entry->access      = (access_byte & (0b111 << 1)) >> 1;
    entry->type        = (bool)(access_byte & (1 << 4));
    entry->privilege   = access_byte & (0b11 << 5);
    entry->present     = (bool)(access_byte & (1 << 7));

    entry->limit_16_19 = (limit & 0xF0000) >> 16;

    entry->zero = 0;
    entry->long_mode   = (bool)(flags & GDT_SEG_LONG);
    entry->size        = (bool)(flags & GDT_SEG_SIZE(1));
    entry->granularity = (bool)(flags & GDT_SEG_GRANULARITY(1));

    entry->base_24_31  = (base & 0xFF000000) >> 24;
}

#define GDT_ENTRIES_CNT 5
static GDTEntry g_gdt[GDT_ENTRIES_CNT];
static GDTR g_gdtr;

_INIT void gdt_init()
{
    /* null segment, needs to be here */
    gdt_encode_entry(0, 0, 0, 0, &g_gdt[0]);
    /* code ring 0 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (GDT_SEG_RX | GDT_SEG_PRIVILEGE(0) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &g_gdt[1]
    );
    /* data ring 0 segment */
    gdt_encode_entry(
        0, 
        0xFFFFF,
        (GDT_SEG_RW | GDT_SEG_PRIVILEGE(0) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &g_gdt[2]
    );
    /* code ring 3 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (GDT_SEG_RX | GDT_SEG_PRIVILEGE(3) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &g_gdt[3]
    );
    /* data ring 3 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (GDT_SEG_RW | GDT_SEG_PRIVILEGE(3) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &g_gdt[4]
    );
    // TODO: add tss

    g_gdtr.base = g_gdt;
    g_gdtr.limit = sizeof(g_gdt) - 1;

    gdt_load(&g_gdtr);
}
