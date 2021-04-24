/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/x86/gdt.h>
#include<stdint.h>
#include<dxgmx/gcc/attrs.h>

/* set to 1 by the cpu if 
 * the segment is accessed,
 * defined just for completeness
 */
#define SEG_ACCESSED     1
/* 1 for data/code segments, 0 otherwise */
#define SEG_TYPE(n) ((n & 1) << 4)
/* ring number 0, 1, 2, 3 */
#define SEG_PRIV_LVL(n)  ((n & 3) << 5)
/* 1 if selector is valid, 0 otherwise */
#define SEG_PRESENT(n)   ((n & 1) << 7)

/* read */
#define SEG_R            0b0
/* read, write */
#define SEG_RW           0b10
/* read, segment grows down */
#define SEG_R_GROW_DOWN  0b100
/* read, write, segment grows down */
#define SEG_RW_GROW_DOWN 0b110
/* execute */
#define SEG_X            0b1000
/* read, execute */
#define SEG_RX           0b1010
/* execute, conforming */
#define SEG_X_CONF       0b1100
/* read, execute, conforming */
#define SEG_RX_CONF      0b1110

/* 0 for byte granularity, 1 for page(4K) granularity */
#define SEG_GRAN(n)      ((n & 1) << 3)
/* 0 for real mode, 1 for protected mode */
#define SEG_SIZE(n)      ((n & 1) << 2)
/* 1 for long mode */
#define SEG_LONG(n)      ((n & 1) << 1)

/*
    lgdt expects a 2 byte limit (size in bytes of the gdt)
    and it's base address in 4 bytes
*/
typedef struct
__ATTR_PACKED
{
    uint16_t limit;
    uint32_t base;
} GDTR;

/*
 * Format of a GDT entry - 64 bits
 * 
 * |63            56|55    52|51    48|47            40|39            32|
 * |----------------+--------+--------+----------------+----------------|
 * |   base 56-63   | flags  |lim16-19|  access byte   |   base 16-23   |
 * |----------------+--------+--------+----------------+----------------|
 * |            base 16-31            |           limit 0-15            |
 * |----------------------------------+---------------------------------|
 * |31                              16|15                              0|
*/
/* defined here as a packed struct for easier access */
typedef struct
__ATTR_PACKED
{
    uint16_t limit_0_15;
    uint16_t base_0_15;
    uint8_t  base_16_23;
    uint8_t  access_byte;
    uint8_t  limit_16_19:4;
    uint8_t  flags:4;
    uint8_t  base_24_31;
} GDTEntry;

#define GDT_ENTRIES_CNT 5
static GDTEntry gdt[GDT_ENTRIES_CNT];

/* this is implemented in loadgtdr.S cuz i don't want to write inline asm */
extern void load_gdtr(GDTR *gdtr);

//note: bit 0 of flags is currently unused

static void gdt_encode_entry(
    uint32_t base,
    uint32_t limit,
    uint8_t access_byte,
    uint8_t flags,
    GDTEntry *target
)
{
    target->limit_0_15  = (limit & 0xFFFF);
    target->base_0_15   = (base & 0xFFFF);

    target->base_16_23  = (base & 0xFF0000) >> 16;
    target->access_byte = access_byte;
    target->limit_16_19 = (limit & 0xF0000) >> 16;
    target->flags       = (flags & 0xF);
    target->base_24_31  = (base & 0xFF000000) >> 24;
}

void gdt_init()
{
    /* null segment, needs to be here */
    gdt_encode_entry(0, 0, 0, 0, &gdt[0]);
    /* code ring 0 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (SEG_RX | SEG_PRIV_LVL(0) | SEG_TYPE(1) | SEG_PRESENT(1)),
        (SEG_GRAN(1) | SEG_SIZE(1) | SEG_LONG(0)),
        &gdt[1]
    );
    /* data ring 0 segment */
    gdt_encode_entry(
        0, 
        0xFFFFF,
        (SEG_RW | SEG_PRIV_LVL(0) | SEG_TYPE(1) | SEG_PRESENT(1)),
        (SEG_GRAN(1) | SEG_SIZE(1) | SEG_LONG(0)),
        &gdt[2]
    );
    /* code ring 3 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (SEG_RX | SEG_PRIV_LVL(3) | SEG_TYPE(1) | SEG_PRESENT(1)),
        (SEG_GRAN(1) | SEG_SIZE(1) | SEG_LONG(0)),
        &gdt[3]
    );
    /* data ring 3 segment */
    gdt_encode_entry(
        0,
        0xFFFFF,
        (SEG_RW | SEG_PRIV_LVL(3) | SEG_TYPE(1) | SEG_PRESENT(1)),
        (SEG_GRAN(1) | SEG_SIZE(1) | SEG_LONG(0)),
        &gdt[4]
    );
    // TODO: add tss

    GDTR gdtr;
    gdtr.base = (uint32_t)gdt;
    gdtr.limit = sizeof(gdt);

    load_gdtr(&gdtr);
}
