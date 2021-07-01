
#ifndef __DXGMX_PAGETABLE_H__
#define __DXGMX_PAGETABLE_H__

#include<stdint.h>
#include<dxgmx/attrs.h>

/*
 * Structure of a page table entry
 * 
 * |31                                    12|11   9|8                        0|
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
 * | physical page base 4 KiB/MiB aligned   |avail.|G |0 |D |A |C |W |U |R |P |
 * |----------------------------------------+------+--+--+--+--+--+--+--+--+--|
*/

/* The page is available in physical memory right now. */
#define _PAGE_PRESENT       (1 << 0)
/* The page is read/write. If not set it's read only. */
#define _PAGE_RW            (1 << 1)
/* 
 * User/Supervisor bit, controls access to the page 
 * based on privilege. If set the page can be accessed by a user 
 * and by the supervisor(kernel), if not only the supervisor can 
 * can access it.
 */
#define _PAGE_USER_ACCESS   (1 << 2)
/* If set write through is enabled, else write back is enabled. */
#define _PAGE_WRITE_THROUGH (1 << 3)
/* Disables caching. */
#define _PAGE_CACHE_DISABLE (1 << 4)
/* 
 * If the page has been accessed (read/written to) this bit
 * will be set.
 */
#define _PAGE_ACCESSED      (1 << 5)
/* If the page has been written to this bit will be set. */
#define _PAGE_DIRTY         (1 << 6)
/* 
 * Prevents the TLB from updating the address for this page 
 * in it's cache if the CR3 register is reset.
 */
#define _PAGE_GLOBAL        (1 << 8)

typedef struct
_ATTR_PACKED S_PageTableEntry
{
    uint32_t base:20;
    uint8_t avail:3;
    uint16_t flags:9;
} PageTableEntry;

typedef struct S_PageTable
{
    PageTableEntry entries[1024];
} PageTable;

/* 0 on success, 1 if 'base' wasn't aligned or 2 if 'flags' was invalid. */
int pagetable_entry_encode(
    uint32_t base,
    uint16_t flags,
    PageTableEntry *entry
);

#endif // __DXGMX_PAGETABLE_H__
