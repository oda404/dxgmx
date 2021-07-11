
#ifndef _DXGMX_PAGING_PAGETABLE_H
#define _DXGMX_PAGING_PAGETABLE_H

#include<stdint.h>
#include<dxgmx/attrs.h>

/*
 * Structure of a page table entry
 * 
 * |31                                    12|11   9|8                        0|
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
 * | physical page base 4 KiB/MiB aligned   |unused|G |0 |D |A |C |W |U |R |P |
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
*/

typedef struct
_ATTR_PACKED S_PageTableEntry
{
    /* This is a 4KiB aligned address to a page frame. */
    uint32_t frame_base: 20;
    /* Free for use :) */
    uint8_t unused: 3;
    /* Set: page will not be deleted from cache if CR3 changes. */
    uint8_t global: 1;
    /**/
    uint8_t pat_memtype: 1;
    /* Set if the page has been written to. */
    uint8_t dirty: 1;
    /* Set if the page has been read/written to. */
    uint8_t accessed: 1;
    /* Set if caching for this page is disabled. */
    uint8_t cache_disabled: 1;
    /* Set if write_through is enabled for this page. */
    uint8_t write_through: 1;
    /* Set if the page can be accessed by users. */
    uint8_t user_access: 1;
    /* Set if the page is read/write. */
    uint8_t rw: 1;
    /* Set if the page is present in physical memory. */
    uint8_t present: 1;
} PageTableEntry;

typedef struct S_PageTable
{
    PageTableEntry entries[1024];
} PageTable;

void pagetable_init(PageTable *table);

#endif //_DXGMX_PAGING_PAGETABLE_H
