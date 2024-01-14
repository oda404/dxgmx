/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/string.h>
#include <dxgmx/x86/gdt.h>

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
    /* If the segment is for long mode. */
    u8 longmode : 1;
    /* 0 for a 16 bit segment, 1 for a 32bit segment. */
    u8 bits32 : 1;
    /* 0 for byte granularity, 1 for 4K */
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

static _INIT void
gdt_encode_entry_base_and_limit(u32 base, u32 limit, GDTEntry* entry)
{
    entry->limit_lo = (limit & 0xFFFF);
    entry->base_lo = (base & 0xFFFFFF);
    entry->limit_hi = (limit & 0xF0000) >> 16;
    entry->base_hi = (base & 0xFF000000) >> 24;
}

extern GDTEntry ___boot_gdt[GDT_ENTRIES_CNT];
extern size_t ___boot_gdt_size;
static TssEntry g_tss;

_INIT void gdt_finish_init()
{
    /* Just in case someone fucks with the gdt in entry.S and doesn't account
     * for it */
    ASSERT(___boot_gdt_size == GDT_ENTRIES_CNT * sizeof(GDTEntry));

    /* Set gdt tss entry */
    GDTEntry* entry = &___boot_gdt[GDT_TSS / sizeof(GDTEntry)];
    gdt_encode_entry_base_and_limit((ptr)&g_tss, sizeof(g_tss), entry);
    entry->accessed = 1; /* For a system segment 1: a TSS and 0: LDT */
    entry->exec = 1;     /* For a TSS 1: 32bit and 0: 16bit */
    entry->present = 1;

    /* Load the gdt */
    memset(&g_tss, 0, sizeof(TssEntry));
    g_tss.ss0 = GDT_KERNEL_DS;
    __asm__ volatile("mov %0, %%ax  \n"
                     "ltr %%ax      \n"
                     :
                     : "i"(GDT_TSS));
}

void tss_set_esp0(ptr esp)
{
    g_tss.esp0 = esp;
}

ptr tss_get_esp0()
{
    return g_tss.esp0;
}
