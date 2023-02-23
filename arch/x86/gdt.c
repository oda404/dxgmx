/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/string.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/gdt.h>

static _INIT void gdt_load(const GDTR* gdtr)
{
    __asm__ volatile("cli                      \n"
                     "lgdt (%0)                \n"
                     "ljmp %1, $reload_segs    \n"
                     "                         \n"
                     "reload_segs:             \n"
                     "  movw %2, %%ax          \n"
                     "  movw %%ax, %%ds        \n"
                     "  movw %%ax, %%es        \n"
                     "  movw %%ax, %%fs        \n"
                     "  movw %%ax, %%gs        \n"
                     "  movw %%ax, %%ss        \n"
                     "  sti                    \n"
                     :
                     : "b"(gdtr), "i"(GDT_KERNEL_CS), "i"(GDT_KERNEL_DS));
}

static _INIT void
gdt_encode_entry_base_and_limit(u32 base, u32 limit, GDTEntry* entry)
{
    entry->limit_lo = (limit & 0xFFFF);
    entry->base_lo = (base & 0xFFFFFF);
    entry->limit_hi = (limit & 0xF0000) >> 16;
    entry->base_hi = (base & 0xFF000000) >> 24;
}

static _INIT void tss_load()
{
    __asm__ volatile("mov %0, %%ax  \n"
                     "ltr %%ax      \n"
                     :
                     : "i"(GDT_TSS));
}

#define GDT_ENTRIES_CNT 6
static GDTEntry g_gdt[GDT_ENTRIES_CNT];
static GDTR g_gdtr;
static TssEntry g_tss;

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
    entry->rw = 1;
    entry->exec = 1;
    entry->dpl = 0;
    entry->code_or_data = 1;
    entry->present = 1;
    entry->granularity = 1;
    entry->bits32 = 1;

    /* data ring 0 segment */
    entry = &g_gdt[DS0_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->rw = 1;
    entry->dpl = 0;
    entry->code_or_data = 1;
    entry->present = 1;
    entry->granularity = 1;
    entry->bits32 = 1;

    /* code ring 3 segment */
    entry = &g_gdt[CS3_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->rw = 1;
    entry->exec = 1;
    entry->dpl = 3;
    entry->code_or_data = 1;
    entry->present = 1;
    entry->granularity = 1;
    entry->bits32 = 1;

    /* data ring 3 segment */
    entry = &g_gdt[DS3_IDX];
    gdt_encode_entry_base_and_limit(0, 0xFFFFF, entry);
    entry->rw = 1;
    entry->dpl = 3;
    entry->code_or_data = 1;
    entry->present = 1;
    entry->granularity = 1;
    entry->bits32 = 1;

    /* tss */
    entry = &g_gdt[TSS_IDX];
    gdt_encode_entry_base_and_limit((ptr)&g_tss, sizeof(g_tss), entry);
    entry->accessed = 1; /* For a system segment 1: a TSS and 0: LDT */
    entry->exec = 1;     /* For a TSS 1: 32bit and 0: 16bit */
    entry->present = 1;

    g_gdtr.base = g_gdt;
    g_gdtr.limit = sizeof(g_gdt) - 1;

    gdt_load(&g_gdtr);
}

_INIT void tss_init()
{
    /* Prepare the TSS */
    memset(&g_tss, 0, sizeof(TssEntry));
    g_tss.ss0 = GDT_KERNEL_DS;

    tss_load();
}

void tss_set_esp0(ptr esp)
{
    g_tss.esp0 = esp;
}
