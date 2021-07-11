/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_GDT_H
#define _DXGMX_X86_GDT_H

#include<dxgmx/attrs.h>
#include<stdint.h>

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
_ATTR_PACKED S_GDTEntry
{
    /* First 16 bits of the limit. */
    uint16_t limit_0_15;
    /* First 16 bits of the base. */
    uint16_t base_0_15;
    /* Bits 16-23 of the base. */
    uint8_t  base_16_23;
    /* Set to one by the cpu if the segment has been accessed. */
    uint8_t  accessed: 1;
    /* How the segment is meant to be accesed. See GDT_SEG(R/RW/X)_(CONF/GROW_DOWN) */
    uint8_t  access: 3;
    /* See GDT_SEG_TYPE. */
    uint8_t  type: 1;
    /* Privilege level of the segment. See GDT_SEG_PRIVILEGE. */
    uint8_t  privilege: 2;
    /* If the segment is valid. */
    uint8_t  present: 1;
    /* Bits 16-19 of the limit. */
    uint8_t  limit_16_19: 4;
    /* Must be 0. */
    uint8_t  zero: 1;
    /* If the segment is for long mode. See GDT_SEG_LONG. */
    uint8_t  long_mode: 1;
    /* See GDT_SEG_SIZE. */
    uint8_t  size: 1;
    /* See GDT_SEG_GRANULARITY */
    uint8_t  granularity: 1;
    /* Bits 24-31 of the base. */
    uint8_t  base_24_31;
} GDTEntry;

typedef struct
_ATTR_PACKED S_GDTR
{
    uint16_t limit;
    GDTEntry *base;
} GDTR;

void gdt_encode_entry(
    uint32_t base,
    uint32_t limit,
    uint8_t access_byte,
    uint8_t flags,
    GDTEntry *gdt
);

void gdt_load(const GDTR *gdtr);

#endif // _DXGMX_X86_GDT_H
