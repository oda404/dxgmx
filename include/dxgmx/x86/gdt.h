/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_X86_GDT_H
#define _DXGMX_X86_GDT_H

#include<dxgmx/attrs.h>
#include<dxgmx/types.h>

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
    u16 limit_0_15;
    /* First 16 bits of the base. */
    u16 base_0_15;
    /* Bits 16-23 of the base. */
    u8  base_16_23;
    /* Set to one by the cpu if the segment has been accessed. */
    u8  accessed: 1;
    /* How the segment is meant to be accesed. See GDT_SEG(R/RW/X)_(CONF/GROW_DOWN) */
    u8  access: 3;
    /* See GDT_SEG_TYPE. */
    u8  type: 1;
    /* Privilege level of the segment. See GDT_SEG_PRIVILEGE. */
    u8  privilege: 2;
    /* If the segment is valid. */
    u8  present: 1;
    /* Bits 16-19 of the limit. */
    u8  limit_16_19: 4;
    /* Must be 0. */
    u8  zero: 1;
    /* If the segment is for long mode. See GDT_SEG_LONG. */
    u8  long_mode: 1;
    /* See GDT_SEG_SIZE. */
    u8  size: 1;
    /* See GDT_SEG_GRANULARITY */
    u8  granularity: 1;
    /* Bits 24-31 of the base. */
    u8  base_24_31;
} GDTEntry;

typedef struct
_ATTR_PACKED S_GDTR
{
    u16 limit;
    GDTEntry *base;
} GDTR;

/* The ring 0 mode code segment */
#define GDT_KERNEL_CS 0x8
/* The ring 0 data segment. */
#define GDT_KERNEL_DS 0x10
#define GDT_USER_CS   0x18
#define GDT_USER_DS   0x20

void gdt_init();

#endif // _DXGMX_X86_GDT_H
