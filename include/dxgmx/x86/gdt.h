
#ifndef __DXGMX_GDT_H__
#define __DXGMX_GDT_H__

#include<stdint.h>
#include<dxgmx/gcc/attrs.h>

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

void gdt_init();
void gdt_encode_entry(
    uint32_t base,
    uint32_t limit,
    uint8_t  access_byte,
    uint8_t  flags,
    GDTEntry *target
);

#endif // __DXGMX_GDT_H__
