/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/x86/gdt.h>
#include<dxgmx/x86/sysgdt.h>
#include<stdint.h>
#include<stdbool.h>

void gdt_load(
    const GDTR *gdtr
)
{
    asm volatile(
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
        : "b"(gdtr), "i"(SYSGDT_KERNEL_CS), "i"(SYSGDT_KERNEL_DS)
    );
}

void gdt_encode_entry(
    uint32_t base,
    uint32_t limit,
    uint8_t access_byte,
    uint8_t flags,
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
