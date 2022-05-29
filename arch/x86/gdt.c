/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/kstack.h>
#include <dxgmx/string.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/gdt.h>

_INIT static void gdt_load(const GDTR* gdtr)
{
    __asm__ volatile("lgdt (%0)                \n"
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
                     : "b"(gdtr), "i"(GDT_KERNEL_CS), "i"(GDT_KERNEL_DS));
}

_INIT static void
gdt_encode_entry_base_and_limit(u32 base, u32 limit, GDTEntry* entry)
{
    entry->limit_0_15 = (limit & 0xFFFF);
    entry->base_0_15 = (base & 0xFFFF);
    entry->base_16_23 = (base & 0xFF0000) >> 16;
    entry->limit_16_19 = (limit & 0xF0000) >> 16;
    entry->base_24_31 = (base & 0xFF000000) >> 24;
}

#define GDT_ENTRIES_CNT 6
static GDTEntry g_gdt[GDT_ENTRIES_CNT];
static GDTR g_gdtr;
static TssEntry g_tss;

_ATTR_NAKED _INIT static void tss_flush()
{
    __asm__ volatile("cli \n"
                     "mov %0, %%ax \n"
                     "ltr %%ax \n"
                     "sti \n"
                     "ret \n"
                     :
                     : "i"(GDT_TSS));
}

_INIT void gdt_init()
{
    memset(g_gdt, 0, sizeof(g_gdt));

#define CS0_IDX (GDT_KERNEL_CS / sizeof(GDTEntry))
#define DS0_IDX (GDT_KERNEL_DS / sizeof(GDTEntry))
#define CS3_IDX (GDT_USER_CS / sizeof(GDTEntry))
#define DS3_IDX (GDT_USER_DS / sizeof(GDTEntry))
#define TSS_IDX (GDT_TSS / sizeof(GDTEntry))

    STATIC_ASSERT(
        (CS0_IDX < GDT_ENTRIES_CNT && DS0_IDX < GDT_ENTRIES_CNT &&
         CS3_IDX < GDT_ENTRIES_CNT && DS3_IDX < GDT_ENTRIES_CNT &&
         TSS_IDX < GDT_ENTRIES_CNT),
        "Out of range GDT segment(s)!");

    STATIC_ASSERT(
        (CS0_IDX != DS0_IDX && DS0_IDX != CS3_IDX && CS3_IDX != DS3_IDX &&
         DS3_IDX != TSS_IDX),
        "Duplicate GDT segments(s)!");

    /* code ring 0 segment */
    GDTEntry* entry = &g_gdt[CS0_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->access_rw = 1;
    entry->access_exec = 1;
    entry->access_dpl = 0;
    entry->access_type = 1;
    entry->access_present = 1;
    entry->granularity = 1;
    entry->size = 1;

    /* data ring 0 segment */
    entry = &g_gdt[DS0_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->access_rw = 1;
    entry->access_dpl = 0;
    entry->access_type = 1;
    entry->access_present = 1;
    entry->granularity = 1;
    entry->size = 1;

    /* code ring 3 segment */
    entry = &g_gdt[CS3_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->access_rw = 1;
    entry->access_exec = 1;
    entry->access_dpl = 3;
    entry->access_type = 1;
    entry->access_present = 1;
    entry->granularity = 1;
    entry->size = 1;

    /* data ring 3 segment */
    entry = &g_gdt[DS3_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->access_rw = 1;
    entry->access_dpl = 3;
    entry->access_type = 1;
    entry->access_present = 1;
    entry->granularity = 1;
    entry->size = 1;

    /* tss */
    entry = &g_gdt[TSS_IDX];
    gdt_encode_entry_base_and_limit((ptr)&g_tss, sizeof(g_tss), entry);
    entry->access_accessed = 1; /* For a system segment 1: a TSS and 0: LDT */
    entry->access_exec = 1;     /* For a TSS 1: 32bit and 0: 16bit */
    entry->access_present = 1;

    g_gdtr.base = g_gdt;
    g_gdtr.limit = sizeof(g_gdt) - 1;

    gdt_load(&g_gdtr);

    /* Prepare the TSS */
    memset(&g_tss, 0, sizeof(TssEntry));
    g_tss.ss0 = GDT_KERNEL_DS;
    /* !! FIXME: create a separate kernel stack. */
    g_tss.esp0 = _kernel_stack_top;

    tss_flush();
}
