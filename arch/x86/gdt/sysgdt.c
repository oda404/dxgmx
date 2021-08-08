/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/sysgdt.h>
#include<dxgmx/x86/gdt.h>

#define GDT_ENTRIES_CNT 5
static GDTEntry gdt[GDT_ENTRIES_CNT];
static GDTR gdtr;

void sysgdt_init()
{ 
    /* null segment, needs to be here */
    gdt_encode_entry(0, 0, 0, 0, &gdt[0]);
    /* code ring 0 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (GDT_SEG_RX | GDT_SEG_PRIVILEGE(0) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &gdt[1]
    );
    /* data ring 0 segment */
    gdt_encode_entry(
        0, 
        0xFFFFF,
        (GDT_SEG_RW | GDT_SEG_PRIVILEGE(0) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &gdt[2]
    );
    /* code ring 3 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (GDT_SEG_RX | GDT_SEG_PRIVILEGE(3) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &gdt[3]
    );
    /* data ring 3 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (GDT_SEG_RW | GDT_SEG_PRIVILEGE(3) | GDT_SEG_TYPE(1) | GDT_SEG_PRESENT),
        (GDT_SEG_GRANULARITY(1) | GDT_SEG_SIZE(1)),
        &gdt[4]
    );
    // TODO: add tss

    gdtr.base = gdt;
    gdtr.limit = sizeof(gdt) - 1;

    gdt_load(&gdtr);
}
